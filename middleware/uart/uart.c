/* 文件名: middleware/uart/uart.c
 * 描述:
 *  UART middleware driver:
 *  - 提供 TX ringbuffer
 *  - 提供 DMA TX 调度与续发
 *  - 覆盖 BSP weak hook，在 DMA 完成时推进 tail
 *
 * 边界:
 *  - 不直接操作具体 UART/DMA 寄存器，寄存器访问由 BSP 提供
 *  - 不做协议解析，不做 framing，不做 command parser
 *  - 当前只完善 TX 路径；RX 以后走 DMA circular + IDLE
 */

/* Private include
 * **************************************************************/
#include "uart.h"
#include "bsp_uart.h"
#include <stdarg.h>
#include <stdint.h>
// #include <stdio.h>
#include <string.h>

/* -----------------------------------------------------------------------------*/
/* Private define
 * **************************************************************/

#ifndef UART_MAX_PORT
#define UART_MAX_PORT 3 /* 你当前只先做 UART1 */
#endif

#ifndef UART_TX_BUF_SIZE
#define UART_TX_BUF_SIZE 256 /* 必须是 2^n */
#endif

#if ((UART_TX_BUF_SIZE & (UART_TX_BUF_SIZE - 1U)) != 0)
#error "UART_TX_BUF_SIZE must be power of two"
#endif

#ifndef UART_RX_DMA_BUF_SIZE
#define UART_RX_DMA_BUF_SIZE 64
#endif

#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE 256
#endif

#if ((UART_RX_BUF_SIZE & (UART_RX_BUF_SIZE - 1U)) != 0)
#error "UART_RX_BUF_SIZE must be power of two"
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define UART_MAX_PORT 3

/* -----------------------------------------------------------------------------*/
/* Private typedef
 * **************************************************************/

/* -----------------------------------------------------------------------------*/
/* Private variable
 * **************************************************************/

uart_drv_t uart_drv;

/* -----------------------------------------------------------------------------*/
/* Private function prototypes
 * **************************************************************/

static uint32_t uart_irq_save(void);
static void uart_irq_restore(uint32_t primask);
static void rb_init(ringbuf_t *rb, uint8_t *buf, uint16_t size);
static uint16_t rb_used_unsafe(const ringbuf_t *rb);
static uint16_t rb_free_unsafe(const ringbuf_t *rb);
static uint16_t rb_write_unsafe(ringbuf_t *rb, const uint8_t *data,
                                uint16_t len);
static uint16_t rb_peek_contig_unsafe(const ringbuf_t *rb, uint8_t **ptr);
static void rb_advance_unsafe(ringbuf_t *rb, uint16_t n);
static void uart_tx_kick(uint8_t port);
static uint16_t rb_read_unsafe(ringbuf_t *rb, uint8_t *data, uint16_t len);
static void itoa(int32_t num, int8_t *str, int32_t base);

/* -----------------------------------------------------------------------------*/
/* Private function definations
 * **************************************************************/

/* 短临界区：
 * 当前并发模型 = 主循环 + DMA ISR
 * 这里用 PRIMASK 简单保护共享状态
 */

static uint32_t uart_irq_save(void) {
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

static void uart_irq_restore(uint32_t primask) {
  if ((primask & 1U) == 0U) {
    __enable_irq();
  }
}

/* ringbuffer init */
static void rb_init(ringbuf_t *rb, uint8_t *buf, uint16_t size) {
  rb->buf = buf;
  rb->size = size;
  rb->mask = (uint16_t)(size - 1U);
  rb->head = 0U;
  rb->tail = 0U;
  rb->overflow_events = 0U;
  rb->overflow_bytes = 0U;
}

/* 下面这些 _unsafe 版本默认要求调用者自行保证并发安全 */

static uint16_t rb_used_unsafe(const ringbuf_t *rb) {
  return (uint16_t)(rb->head - rb->tail);
}

static uint16_t rb_free_unsafe(const ringbuf_t *rb) {
  return (uint16_t)(rb->size - rb_used_unsafe(rb));
}

/* 写 ringbuffer：
 * - 空间不够则截断
 * - 记录 overflow_events / overflow_bytes
 */
static uint16_t rb_write_unsafe(ringbuf_t *rb, const uint8_t *data,
                                uint16_t len) {
  uint16_t free;
  uint16_t want;
  uint16_t head;
  uint16_t idx;
  uint16_t first;

  free = rb_free_unsafe(rb);
  want = len;

  if (len > free) {
    len = free;
    rb->overflow_events++;
    rb->overflow_bytes += (uint32_t)(want - len);
  }

  if (len == 0U) {
    return 0U;
  }

  head = rb->head;
  idx = (uint16_t)(head & rb->mask);

  first = (uint16_t)(rb->size - idx);
  first = MIN(first, len);

  memcpy(&rb->buf[idx], data, first);

  if (len > first) {
    memcpy(&rb->buf[0], data + first, (uint16_t)(len - first));
  }

  rb->head = (uint16_t)(head + len);
  return len;
}

/* 取从 tail 开始的一段连续区间，用于一次 DMA TX */
static uint16_t rb_peek_contig_unsafe(const ringbuf_t *rb, uint8_t **ptr) {
  uint16_t used;
  uint16_t tail;
  uint16_t idx;
  uint16_t contig;

  used = rb_used_unsafe(rb);
  if (used == 0U) {
    *ptr = 0;
    return 0U;
  }

  tail = rb->tail;
  idx = (uint16_t)(tail & rb->mask);

  contig = (uint16_t)(rb->size - idx);
  contig = MIN(contig, used);

  *ptr = &rb->buf[idx];
  return contig;
}

static void rb_advance_unsafe(ringbuf_t *rb, uint16_t n) {
  uint16_t used = rb_used_unsafe(rb);

  /* 防御式处理，避免 tail 推过头 */
  if (n > used) {
    n = used;
  }

  rb->tail = (uint16_t)(rb->tail + n);
}

/* kick 逻辑：
 * 1) 先在临界区里判断是否可发，并拿到本次连续块
 * 2) 先占住软件状态 tx_busy / last_dma_len
 * 3) 出临界区后启动 DMA
 * 4) 若启动失败，则回滚软件状态
 */
static void uart_tx_kick(uint8_t port) {
  uart_dev_t *u;
  uint8_t *p;
  uint16_t n;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  p = 0;
  n = 0U;

  if (!u->inited) {
    return;
  }

  if (!u->use_dma_tx) {
    return;
  }

  key = uart_irq_save();

  /* 软件 busy + 硬件 busy 双判定 */
  if (u->tx_busy || bsp_uart_dma_tx_busy(port)) {
    uart_irq_restore(key);
    return;
  }

  n = rb_peek_contig_unsafe(&u->tx_rb, &p);
  if ((n == 0U) || (p == 0)) {
    uart_irq_restore(key);
    return;
  }

  /* 在临界区里先占状态，防止主循环/ISR 重入 kick */
  u->tx_busy = 1U;
  u->last_dma_len = n;

  uart_irq_restore(key);

  /* 真正启动 DMA */
  if (bsp_uart_dma_tx_start(port, p, n) != 0) {
    /* 启动失败，回滚状态 */
    key = uart_irq_save();
    u->tx_busy = 0U;
    u->last_dma_len = 0U;
    uart_irq_restore(key);
  }
}

static uint16_t rb_read_unsafe(ringbuf_t *rb, uint8_t *data, uint16_t len) {
  uint16_t used;
  uint16_t tail;
  uint16_t idx;
  uint16_t first;

  used = rb_used_unsafe(rb);
  if (len > used) {
    len = used;
  }

  if ((len == 0U) || (data == 0)) {
    return 0U;
  }

  tail = rb->tail;
  idx = (uint16_t)(tail & rb->mask);

  first = (uint16_t)(rb->size - idx);
  first = MIN(first, len);

  memcpy(data, &rb->buf[idx], first);

  if (len > first) {
    memcpy(data + first, &rb->buf[0], (uint16_t)(len - first));
  }

  rb->tail = (uint16_t)(tail + len);
  return len;
}

static void itoa(int32_t num, int8_t *str, int32_t base) {
  int32_t i = 0;
  int32_t isNegative = 0;
  uint32_t n;
  uint32_t rem;

  if (base < 2 || base > 16) {
    str[0] = '\0';
    return;
  }

  /* 只有十进制才处理负号 */
  if ((base == 10) && (num < 0)) {
    isNegative = 1;
    n = (uint32_t)(-num);
  } else {
    n = (uint32_t)num;
  }

  do {
    rem = n % (uint32_t)base;
    if (rem < 10) {
      str[i++] = (int8_t)(rem + '0');
    } else {
      str[i++] = (int8_t)(rem - 10 + 'a'); /* 小写十六进制 */
    }
    n /= (uint32_t)base;
  } while (n != 0U);

  if (isNegative) {
    str[i++] = '-';
  }

  str[i] = '\0';

  /* reverse */
  for (int32_t j = 0, k = i - 1; j < k; j++, k--) {
    int8_t temp = str[j];
    str[j] = str[k];
    str[k] = temp;
  }
}

static void uitoa(uint32_t num, int8_t *str, int32_t base) {
  int32_t i = 0;
  uint32_t rem;

  if (base < 2 || base > 16) {
    str[0] = '\0';
    return;
  }

  do {
    rem = num % (uint32_t)base;
    if (rem < 10U) {
      str[i++] = (int8_t)(rem + '0');
    } else {
      str[i++] = (int8_t)(rem - 10U + 'a');
    }
    num /= (uint32_t)base;
  } while (num != 0U);

  str[i] = '\0';

  for (int32_t j = 0, k = i - 1; j < k; j++, k--) {
    int8_t temp = str[j];
    str[j] = str[k];
    str[k] = temp;
  }
}

static int32_t vsprintf(char *str, const char *format, va_list args) {
  int32_t i = 0;
  const char *ptr;
  int8_t buffer[32];

  for (ptr = format; *ptr != '\0'; ptr++) {
    if (*ptr == '%') {
      ptr++;

      switch (*ptr) {
      case 'd':
        itoa(va_arg(args, int32_t), buffer, 10);
        for (int32_t j = 0; buffer[j] != '\0'; j++) {
          str[i++] = buffer[j];
        }
        break;

      case 'x':
        uitoa(va_arg(args, uint32_t), buffer, 16);
        for (int32_t j = 0; buffer[j] != '\0'; j++) {
          str[i++] = buffer[j];
        }
        break;

      case 's': {
        int8_t *arg_str = va_arg(args, int8_t *);
        if (arg_str == 0) {
          arg_str = (int8_t *)"(null)";
        }
        while (*arg_str) {
          str[i++] = *arg_str++;
        }
      } break;

      case '%':
        str[i++] = '%';
        break;

      default:
        str[i++] = '%';
        str[i++] = *ptr;
        break;
      }
    } else {
      str[i++] = *ptr;
    }
  }

  str[i] = '\0';
  return i;
}

/* -----------------------------------------------------------------------------*/
/* Public APIs
 * **************************************************************/

/* -----------------------------------------------------------------------------*/
/* UART Initialize
 * **************************************************************/

void uart_init(uint8_t port, const uart_cfg_t *cfg) {
  uart_dev_t *u;
  uint32_t baud;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (u->inited) {
    return;
  }

  baud = (cfg && cfg->baudrate) ? cfg->baudrate : 115200U;
  u->use_dma_tx = (cfg && cfg->use_dma_tx) ? 1U : 0U;

  /* 先初始化底层 BSP */
  bsp_uart_init(port, baud);

  rb_init(&u->tx_rb, u->tx_buf, UART_TX_BUF_SIZE); // 发送缓冲区
  rb_init(&u->rx_rb, u->rx_buf, UART_RX_BUF_SIZE); // 接收缓冲区

  /* 设置初始状态 */
  u->tx_busy = 0U;
  u->last_dma_len = 0U;
  u->rx_dma_last_pos = 0U;
  u->rx_suspended = 0U;

  /* 初始化 RX DMA 缓冲区 */
  bsp_uart_dma_init(port, u->rx_dma_buf, UART_RX_DMA_BUF_SIZE);

  /* 标记为已初始化 */
  u->inited = 1U;
}

/* -----------------------------------------------------------------------------*/
/* UART TX API
 * **************************************************************/

/* 非阻塞写：
 * - 只把数据写入 TX ringbuffer
 * - 然后尝试启动 DMA
 * - 返回实际入队字节数
 */
size_t uart_write(uint8_t port, const uint8_t *data, size_t len) {
  uart_dev_t *u;
  uint16_t w;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if ((data == 0) || (len == 0U)) {
    return 0U;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return 0U;
  }

  /* 当前 ring index 是 uint16_t，单次写入上限收一下 */
  if (len > 0xFFFFU) {
    len = 0xFFFFU;
  }

  key = uart_irq_save();
  w = rb_write_unsafe(&u->tx_rb, data, (uint16_t)len);
  uart_irq_restore(key);

  /* 尝试 kick DMA */
  uart_tx_kick(port);

  return (size_t)w;
}

/* 轮询单字节发送 */
void uart_putc_poll(uint8_t port, uint8_t ch) { (void)bsp_uart_putc(port, ch); }

size_t uart_tx_pending(uint8_t port) {
  size_t used;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  used = (size_t)rb_used_unsafe(&uart_drv.dev[port].tx_rb);
  uart_irq_restore(key);

  return used;
}

size_t uart_tx_free(uint8_t port) {
  size_t free;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  free = (size_t)rb_free_unsafe(&uart_drv.dev[port].tx_rb);
  uart_irq_restore(key);

  return free;
}

uint32_t uart_tx_overflow_events(uint8_t port) {
  uint32_t v;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  v = uart_drv.dev[port].tx_rb.overflow_events;
  uart_irq_restore(key);

  return v;
}

uint32_t uart_tx_overflow_bytes(uint8_t port) {
  uint32_t v;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  v = uart_drv.dev[port].tx_rb.overflow_bytes;
  uart_irq_restore(key);

  return v;
}

/* Name: uart_tx_clear_stats
 * Params:
 * Return Val:
 * Description:
 */
void uart_tx_clear_stats(uint8_t port) {
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  if (!uart_drv.dev[port].inited) {
    return;
  }

  key = uart_irq_save();
  uart_drv.dev[port].tx_rb.overflow_events = 0U;
  uart_drv.dev[port].tx_rb.overflow_bytes = 0U;
  uart_irq_restore(key);
}

/* Name: my_printf
 * Params:
 * Return Val:
 * Description:
 */
size_t my_printf(uint8_t port, const char *format, ...) {
  char buffer[256];
  va_list args;

  va_start(args, format);

  // size_t len = 0;
  size_t len = vsprintf(buffer, format, args);

  va_end(args);

  if (len > 0) {
    uart_write(port, (uint8_t *)buffer, len);
  }
  return len;
}

/* -----------------------------------------------------------------------------*/
/* UART RX API
 * **************************************************************/

/* Name: uart_rx_available
 * Params: uart_port
 * Return Val: (uint16_t)(rb->head - rb->tail);
 * Description: 查看当前UART Device's rx_rb里有多少字节
 */
size_t uart_rx_available(uint8_t port) {
  size_t used;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  used = (size_t)rb_used_unsafe(&uart_drv.dev[port].rx_rb);
  uart_irq_restore(key);

  return used;
}

/* Name: uart_rx_poll
 * Params:
 * Return Val:
 * Description: uart_rx_poll() 的职责，是把 DMA circular 缓冲里的新字节搬到软件
 * rx_rb 里。 它会读当前 DMA 写指针 pos，和上次位置 rx_dma_last_pos
 * 比较，然后把新增数据写进 u->rx_rb， 最后更新 rx_dma_last_pos。
 */
void uart_rx_poll(uint8_t port) {
  uart_dev_t *u;
  uint16_t pos;
  uint16_t last;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return;
  }

  pos = bsp_uart_dma_rx_pos(port, UART_RX_DMA_BUF_SIZE);

  key = uart_irq_save();
  last = u->rx_dma_last_pos;

  if (pos == last) {
    uart_irq_restore(key);
    return;
  }

  if (pos > last) {
    rb_write_unsafe(&u->rx_rb, &u->rx_dma_buf[last], (uint16_t)(pos - last));
  } else {
    rb_write_unsafe(&u->rx_rb, &u->rx_dma_buf[last],
                    (uint16_t)(UART_RX_DMA_BUF_SIZE - last));

    if (pos > 0U) {
      rb_write_unsafe(&u->rx_rb, &u->rx_dma_buf[0], pos);
    }
  }

  u->rx_dma_last_pos = pos;
  uart_irq_restore(key);
}

/* Name: uart_rx_getc
 * Params:
 * Return Val:
 * Description: 读1个字节并消费
 */
int uart_rx_getc(uint8_t port, uint8_t *ch) {
  uint16_t n;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return -1;
  }

  if ((ch == 0) || (!uart_drv.dev[port].inited)) {
    return -1;
  }

  key = uart_irq_save();
  n = rb_read_unsafe(&uart_drv.dev[port].rx_rb, ch, 1U);
  uart_irq_restore(key);

  return (n == 1U) ? 0 : -1;
}

/* Name: uart_rx_read
 * Params:
 * Return Val:
 * Description: 读长度为len的一段字节并消费
 */
size_t uart_rx_read(uint8_t port, uint8_t *buf, size_t len) {
  uint16_t n;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if ((buf == 0) || (len == 0U) || (!uart_drv.dev[port].inited)) {
    return 0U;
  }

  if (len > 0xFFFFU) {
    len = 0xFFFFU;
  }

  key = uart_irq_save();
  n = rb_read_unsafe(&uart_drv.dev[port].rx_rb, buf, (uint16_t)len);
  uart_irq_restore(key);

  return (size_t)n;
}

/* Name: uart_rx_peek
 * Params:
 *   port   - UART端口号
 *   offset - 相对当前RX读指针tail的偏移量
 *   ch     - 输出参数，用于返回读取到的字节
 * Return Val:
 *   1 - 读取成功
 *   0 - 读取失败（端口非法 / 未初始化 / offset越界 / ch为空）
 * Description:
 *   从UART软件RX ringbuffer中按偏移读取1个字节，但不消费数据，
 *   不推进tail，适用于协议层查看帧头、长度、命令字等场景。
 */
uint8_t uart_rx_peek(uint8_t port, uint16_t offset, uint8_t *ch) {
  uart_dev_t *u;
  uint32_t key;
  uint16_t used;
  uint16_t index;

  if (port >= UART_MAX_PORT || ch == 0) {
    return 0U;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return 0U;
  }

  key = uart_irq_save();

  used = rb_used_unsafe(&u->rx_rb);
  if (offset >= used) {
    uart_irq_restore(key);
    return 0U;
  }

  index = (uint16_t)((u->rx_rb.tail + offset) & u->rx_rb.mask);
  *ch = u->rx_rb.buf[index];

  uart_irq_restore(key);
  return 1U;
}

/* Name: uart_rx_drop
 * Params:
 *   port - UART端口号
 *   len  - 需要丢弃的字节数
 * Return Val:
 *   None
 * Description:
 *   从UART软件RX ringbuffer中丢弃指定长度的数据，
 *   本质上是推进RX读指针tail，不拷贝数据。
 *   若len大于当前可用数据长度，则按当前可用长度处理。
 *   适用于协议层在完成解析或跳过无效字节时消费接收数据。
 */
void uart_rx_drop(uint8_t port, uint16_t len) {
  uart_dev_t *u;
  uint32_t key;
  uint16_t used;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return;
  }

  key = uart_irq_save();

  used = rb_used_unsafe(&u->rx_rb);
  if (len > used) {
    len = used;
  }

  u->rx_rb.tail = (uint16_t)(u->rx_rb.tail + len);

  uart_irq_restore(key);
}

uint32_t uart_rx_overflow_events(uint8_t port) {
  uint32_t v;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  v = uart_drv.dev[port].rx_rb.overflow_events;
  uart_irq_restore(key);

  return v;
}

uint32_t uart_rx_overflow_bytes(uint8_t port) {
  uint32_t v;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return 0U;
  }

  if (!uart_drv.dev[port].inited) {
    return 0U;
  }

  key = uart_irq_save();
  v = uart_drv.dev[port].rx_rb.overflow_bytes;
  uart_irq_restore(key);

  return v;
}

void uart_rx_clear_stats(uint8_t port) {
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  if (!uart_drv.dev[port].inited) {
    return;
  }

  key = uart_irq_save();
  uart_drv.dev[port].rx_rb.overflow_events = 0U;
  uart_drv.dev[port].rx_rb.overflow_bytes = 0U;
  uart_irq_restore(key);
}

void uart_rx_reset(uint8_t port) {
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  if (!uart_drv.dev[port].inited) {
    return;
  }

  key = uart_irq_save();

  /* 1) 清软件 ringbuffer */
  uart_drv.dev[port].rx_rb.head = 0U;
  uart_drv.dev[port].rx_rb.tail = 0U;
  uart_drv.dev[port].rx_rb.overflow_events = 0U;
  uart_drv.dev[port].rx_rb.overflow_bytes = 0U;

  /* 2) 清 DMA 缓冲 */
  memset(uart_drv.dev[port].rx_dma_buf, 0, UART_RX_DMA_BUF_SIZE);

  /* 3) 清软件游标 */
  uart_drv.dev[port].rx_dma_last_pos = 0U;

  uart_irq_restore(key);

  /* 4) 通知 BSP 重同步 UART RX DMA */
  bsp_uart_rx_resync(port, uart_drv.dev[port].rx_dma_buf, UART_RX_DMA_BUF_SIZE);
}

void uart_rx_suspend(uint8_t port) {
  uint32_t key;
  uart_dev_t *u;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return;
  }

  /* 1) 先停硬件 RX 路径 */
  bsp_uart_rx_suspend(port);

  /* 2) 再更新软件状态 */
  key = uart_irq_save();

  u->rx_suspended = 1U;

  /* 清软件接收状态，避免恢复后还读到旧数据 */
  u->rx_rb.head = 0U;
  u->rx_rb.tail = 0U;
  u->rx_rb.overflow_events = 0U;
  u->rx_rb.overflow_bytes = 0U;
  u->rx_dma_last_pos = 0U;

  /* 可选：调试阶段建议清 DMA 缓冲，方便观察 */
  memset(u->rx_dma_buf, 0, UART_RX_DMA_BUF_SIZE);

  uart_irq_restore(key);
}

void uart_rx_resume(uint8_t port) {
  uint32_t key;
  uart_dev_t *u;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return;
  }

  key = uart_irq_save();

  /* 先清软件接收状态 */
  u->rx_rb.head = 0U;
  u->rx_rb.tail = 0U;
  u->rx_rb.overflow_events = 0U;
  u->rx_rb.overflow_bytes = 0U;
  u->rx_dma_last_pos = 0U;

  /* 可选：调试阶段建议清 DMA 缓冲 */
  memset(u->rx_dma_buf, 0, UART_RX_DMA_BUF_SIZE);

  uart_irq_restore(key);

  /* 重同步硬件 RX DMA 路径 */
  bsp_uart_rx_resync(port, u->rx_dma_buf, UART_RX_DMA_BUF_SIZE);

  key = uart_irq_save();
  u->rx_suspended = 0U;
  uart_irq_restore(key);
}
/* -----------------------------------------------------------------------------*/
/* BSP weak hook override
 * **************************************************************/

/* DMA TX 完成中断回调：
 * - 推进 tail
 * - 清理软件 busy 状态
 * - 尝试续发下一段
 */
void bsp_uart_dma_tx_done_isr(uint8_t port) {
  uart_dev_t *u;
  uint16_t done_len;
  uint32_t key;

  if (port >= UART_MAX_PORT) {
    return;
  }

  u = &uart_drv.dev[port];
  if (!u->inited) {
    return;
  }

  key = uart_irq_save();

  done_len = u->last_dma_len;
  rb_advance_unsafe(&u->tx_rb, done_len);

  u->last_dma_len = 0U;
  u->tx_busy = 0U;

  uart_irq_restore(key);

  /* 继续发下一段 */
  uart_tx_kick(port);
}

void bsp_uart_rx_idle_isr(uint8_t port) { uart_rx_poll(port); }
