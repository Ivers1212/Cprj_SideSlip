/**
 ******************************************************************************
 * @file    uart.h
 * @author  Ivers
 * @brief   UART driver interface (bare-metal)
 *
 * @details
 *   - 提供统一 UART 接口（轮询/中断/DMA 可切换）
 *   - 不包含板级资源绑定（那属于 bsp）
 *   - 应用层只包含本头文件
 *
 ******************************************************************************
 */

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private include
 * **************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* -----------------------------------------------------------------------------*/
/* Public define
 * **************************************************************/

#define UART_MAX_PORT 3 /* 你当前只先做 UART1 */

#define UART_TX_BUF_SIZE 256 /* 必须是 2^n */

#define UART_RX_DMA_BUF_SIZE 64

#define UART_RX_BUF_SIZE 256

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define UART_MAX_PORT 3

/* -----------------------------------------------------------------------------*/
/* Public typedef
 * **************************************************************/

typedef struct {
  uint32_t baudrate;
  uint8_t use_dma_tx; /* 0: poll/primitive only, 1: DMA TX + ringbuffer */
} uart_cfg_t;

typedef struct {
  uint8_t *buf;
  uint16_t size; /* must be 2^n */
  uint16_t mask; /* size - 1 */

  volatile uint16_t head; /* monotonic producer index */
  volatile uint16_t tail; /* monotonic consumer index */

  volatile uint32_t overflow_events;
  volatile uint32_t overflow_bytes;
} ringbuf_t;

typedef struct {
  uint8_t inited;     /* 标识UART 是否初始化*/
  uint8_t use_dma_tx; /* 是否使用DMA TX */
  uint32_t baudrate;

  /* Tx管理 */
  ringbuf_t tx_rb;
  volatile uint8_t tx_busy;       /* 软件语义：当前有一笔 DMA TX 在飞 */
  volatile uint16_t last_dma_len; /* 当前这笔 DMA 实际提交长度 */

  /* Rx DMA circular + software ring */
  uint8_t rx_suspended;
  ringbuf_t rx_rb;
  volatile uint16_t rx_dma_last_pos;

  /* buffer */
  uint8_t tx_buf[UART_TX_BUF_SIZE];
  uint8_t rx_buf[UART_RX_BUF_SIZE];
  uint8_t rx_dma_buf[UART_RX_DMA_BUF_SIZE];
} uart_dev_t; // 每个UART端口的设备结构体
              //
typedef struct {
  uart_dev_t dev[UART_MAX_PORT];
} uart_drv_t; // 管理多个UART端口的驱动结构体

/* -----------------------------------------------------------------------------*/
/* Public variable
 * **************************************************************/

extern uart_drv_t uart_drv;

/* -----------------------------------------------------------------------------*/
/* Public APIs
 * **************************************************************/

/* 初始化 UART driver */
void uart_init(uint8_t port, const uart_cfg_t *cfg);

/* 非阻塞写:
 * - 返回实际写入 ringbuffer 的字节数
 * - 若 buffer 空间不足，会截断写入
 */
size_t uart_write(uint8_t port, const uint8_t *data, size_t len);

/* 轮询单字节发送（早期 bring-up / 调试用） */
void uart_putc_poll(uint8_t port, uint8_t ch);

/* UART_TX调试/状态查询 */
size_t uart_tx_pending(uint8_t port);           /* 当前待发送字节数 */
size_t uart_tx_free(uint8_t port);              /* 当前剩余可写空间 */
uint32_t uart_tx_overflow_events(uint8_t port); /* 溢出次数 */
uint32_t uart_tx_overflow_bytes(uint8_t port);  /* 累计丢弃字节数 */

/* RX 搬运/读取接口*/
size_t uart_rx_available(uint8_t port);
void uart_rx_poll(uint8_t port);
int uart_rx_getc(uint8_t port, uint8_t *ch);
size_t uart_rx_read(uint8_t port, uint8_t *buf, size_t len);

/* RX ringbuf 管理*/
uint8_t uart_rx_peek(uint8_t port, uint16_t offset, uint8_t *ch);
void uart_rx_drop(uint8_t port, uint16_t len);

/* 可选：清零统计 */
void uart_tx_clear_stats(uint8_t port);
uint32_t uart_rx_overflow_events(uint8_t port);
uint32_t uart_rx_overflow_bytes(uint8_t port);
void uart_rx_clear_stats(uint8_t port);

/* uart rx 流程管控 */
void uart_rx_reset(uint8_t port);
void uart_rx_suspend(uint8_t port);
void uart_rx_resume(uint8_t port);

/* 由 BSP 的 DMA IRQ 间接回调到这里（覆盖 BSP weak hook） */
void bsp_uart_dma_tx_done_isr(uint8_t port);
void bsp_uart_rx_idle_isr(uint8_t port);

/* 调试使用打印函数 */
size_t my_printf(uint8_t port, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
