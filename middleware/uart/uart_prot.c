/* 文件名: uart_prot.c
 * 描述:
 *   UART protocol parser:
 *   - 基于 uart_rx_peek()/uart_rx_drop() 从软件 RX ringbuffer 中流式解析固定帧
 *   - 当前协议为固定 16 字节命令帧
 *
 * 边界:
 * 1. 本模块只负责“找帧、校验、提取字段、回调业务钩子”。
 * 2. 不直接操作 UART 内部 ringbuffer 结构，不直接修改 rx_rb.head/tail。
 * 3. ringbuffer 的存取由 uart.c 提供的公共接口完成。
 */

/* Private includes
 * **************************************************************/
#include "uart_prot.h"
#include "uart.h"
#include <stdint.h>
#include <string.h>

/* -----------------------------------------------------------------------------*/
/* Private define
 * **************************************************************/
#ifndef UART_FRAME_SIZE
#define UART_FRAME_SIZE 16U
#endif

#ifndef UART_SYNC_CODE1
#define UART_SYNC_CODE1 0xAAU
#endif

#ifndef UART_SYNC_CODE2
#define UART_SYNC_CODE2 0x55U
#endif

#ifndef UART_FIXED_LEN_MARK
#define UART_FIXED_LEN_MARK 0x02U
#endif

#ifndef UART_PROTO_CMD_BRIGHTNESS
#define UART_PROTO_CMD_BRIGHTNESS 0x82U
#endif

#ifndef UART_PROTO_CMD_BIT
#define UART_PROTO_CMD_BIT 0x83U
#endif

#ifndef UART_PROTO_CMD_SOURCE
#define UART_PROTO_CMD_SOURCE 0x84U
#endif

#ifndef UART_PROTO_CMD_MFD
#define UART_PROTO_CMD_MFD 0xA0U
#endif

#ifndef UART_PROT_DBG_PORT
#define UART_PROT_DBG_PORT 2U
#endif

/* 解析错误码 */
#define UART_PROT_ERR_PORT (-1)
#define UART_PROT_ERR_NULL (-2)
#define UART_PROT_ERR_HEADER (-3)
#define UART_PROT_ERR_CMD (-4)
#define UART_PROT_ERR_LEN_MARK (-5)
#define UART_PROT_ERR_CMD_REPEAT (-6)
#define UART_PROT_ERR_CHECKSUM (-7)

/* -----------------------------------------------------------------------------*/
/* Private typedef
 * **************************************************************/

/* -----------------------------------------------------------------------------*/
/* Private variable
 * **************************************************************/

/* -----------------------------------------------------------------------------*/
/* Private function prototypes
 * **************************************************************/
static void uart_send_debug(const uint8_t *data, uint16_t len);
static uint8_t uart_protocol_is_valid_cmd(uint8_t cmd);
static uint16_t uart_protocol_calc_checksum(const uint8_t *frame);
static uint8_t uart_protocol_peek_buf(uint8_t port, uint16_t offset,
                                      uint8_t *buf, uint16_t len);
static int uart_protocol_try_parse(uint8_t port, uart_protocol_t *protocol);

/* -----------------------------------------------------------------------------*/
/* Private function definitions
 * **************************************************************/
static void uart_send_debug(const uint8_t *data, uint16_t len) {
  if ((data == 0) || (len == 0U)) {
    return;
  }
  uart_write(UART_PROT_DBG_PORT, data, len);
}

static uint8_t uart_protocol_is_valid_cmd(uint8_t cmd) {
  if ((cmd == UART_PROTO_CMD_BRIGHTNESS) || (cmd == UART_PROTO_CMD_BIT) ||
      (cmd == UART_PROTO_CMD_SOURCE) || (cmd == UART_PROTO_CMD_MFD)) {
    return 1U;
  }
  return 0U;
}

static uint16_t uart_protocol_calc_checksum(const uint8_t *frame) {
  uint16_t sum = 0U;
  uint16_t i;

  for (i = 0U; i < 14U; i++) {
    sum = (uint16_t)(sum + frame[i]);
  }

  return sum;
}

/* 从 RX ringbuffer 的 tail+offset 开始，连续窥视 len 字节到 buf 中，不消费数据
 */
static uint8_t uart_protocol_peek_buf(uint8_t port, uint16_t offset,
                                      uint8_t *buf, uint16_t len) {
  uint16_t i;
  uint8_t ch;

  if (buf == 0) {
    return 0U;
  }

  for (i = 0U; i < len; i++) {
    if (!uart_rx_peek(port, (uint16_t)(offset + i), &ch)) {
      return 0U;
    }
    buf[i] = ch;
  }

  return 1U;
}

/* 尝试解析 1 帧
 * 返回值:
 *   1  : 成功解析 1 帧，且已从 RX ringbuffer 中消费掉该帧
 *   0  : 当前数据不足以组成 1 帧，等待更多数据
 *   <0 : 遇到无效帧，已做最小重同步处理（通常 drop 1 字节）
 */
static int uart_protocol_try_parse(uint8_t port, uart_protocol_t *protocol) {
  uint8_t frame[UART_FRAME_SIZE];
  uint16_t sum;
  uint16_t recv_sum;

  if (port >= UART_MAX_PORT) {
    return UART_PROT_ERR_PORT;
  }

  if (protocol == 0) {
    return UART_PROT_ERR_NULL;
  }

  /* 至少先有 2 字节，才能找帧头 */
  while (uart_rx_available(port) >= 2U) {
    uint8_t b0;
    uint8_t b1;

    if (!uart_rx_peek(port, 0U, &b0)) {
      return 0;
    }
    if (!uart_rx_peek(port, 1U, &b1)) {
      return 0;
    }

    if ((b0 == UART_SYNC_CODE1) && (b1 == UART_SYNC_CODE2)) {
      break;
    }

    /* 不是帧头，丢 1 字节继续找同步 */
    uart_rx_drop(port, 1U);
  }

  /* 找到帧头后，数据还不够一整帧 */
  if (uart_rx_available(port) < UART_FRAME_SIZE) {
    return 0;
  }

  if (!uart_protocol_peek_buf(port, 0U, frame, UART_FRAME_SIZE)) {
    return 0;
  }

  /* Byte0~1: 帧头 */
  if ((frame[0] != UART_SYNC_CODE1) || (frame[1] != UART_SYNC_CODE2)) {
    uart_rx_drop(port, 1U);
    return UART_PROT_ERR_HEADER;
  }

  /* Byte3: 命令码 */
  if (!uart_protocol_is_valid_cmd(frame[3])) {
    uart_rx_drop(port, 1U);
    return UART_PROT_ERR_CMD;
  }

  /* Byte11: 固定 0x02 */
  if (frame[11] != UART_FIXED_LEN_MARK) {
    uart_rx_drop(port, 1U);
    return UART_PROT_ERR_LEN_MARK;
  }

  /* Byte12: 命令码重复 */
  if (frame[12] != frame[3]) {
    uart_rx_drop(port, 1U);
    return UART_PROT_ERR_CMD_REPEAT;
  }

  /* Byte14~15: checksum_hi / checksum_lo */
  sum = uart_protocol_calc_checksum(frame);
  recv_sum = (uint16_t)(((uint16_t)frame[14] << 8) | frame[15]);

  if (sum != recv_sum) {
    uart_rx_drop(port, 1U);
    return UART_PROT_ERR_CHECKSUM;
  }

  /* 提取协议结构体
   * header   = Byte0~1
   * data     = Byte2~13
   * checksum = Byte14~15
   */
  memcpy(protocol->header, &frame[0], UART_HEADER_SIZE);
  memcpy(protocol->data, &frame[2], UART_DATA_SIZE);
  protocol->checksum_type = CHECKSUM_CUSTOM;
  protocol->checksum[0] = frame[14];
  protocol->checksum[1] = frame[15];

  /* 整帧消费掉 */
  uart_rx_drop(port, UART_FRAME_SIZE);
  return 1;
}

/* -----------------------------------------------------------------------------*/
/* Public APIs
 * **************************************************************/

void uart_protocol_rx_poll(uint8_t port, uart_protocol_t *protocol) {
  int result;

  if (port >= UART_MAX_PORT) {
    return;
  }

  if (protocol == 0) {
    return;
  }

  while (1) {
    result = uart_protocol_try_parse(port, protocol);

    if (result > 0) {
      handle_uart_protocol(port, protocol);
      continue;
    }

    if (result < 0) {
      handle_protocol_error(port, result);
      continue;
    }

    /* result == 0: 当前没有足够数据，退出本轮 */
    break;
  }
}

/* -----------------------------------------------------------------------------*/
/* Weak hook override
 * **************************************************************/
__attribute__((weak)) void handle_uart_protocol(uint8_t port,
                                                uart_protocol_t *protocol) {
  (void)port;

  /* 默认调试输出：原样打出解析后的 header/data/checksum */
  uart_send_debug(protocol->header, UART_HEADER_SIZE);
  uart_send_debug(protocol->data, UART_DATA_SIZE);
  uart_send_debug(protocol->checksum, UART_CHECK_SIZE);
}

__attribute__((weak)) void handle_protocol_error(uint8_t port, int error_code) {
  uint8_t msg[5];

  (void)port;

  msg[0] = 0xFFU;
  msg[1] = 0xFFU;
  msg[2] = 0xFEU;
  msg[3] = (uint8_t)error_code;
  msg[4] = 0x0AU;

  uart_send_debug(msg, (uint16_t)sizeof(msg));
}
