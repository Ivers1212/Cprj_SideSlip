#ifndef BSP_UART_H
#define BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f103xb.h"
#include "stm32f1xx.h"
#include <stdint.h>

/* -----------------------------------------------------------------------------*/
/* Public BSP APIs
 * **************************************************************/

/* UART base init */
void bsp_uart_init(uint8_t port, uint32_t baudrate);
void bsp_uart_dma_init(uint8_t port, uint8_t *rx_buf, uint16_t rx_len);

/* polling primitives */
int bsp_uart_putc(uint8_t port, uint8_t ch);
int bsp_uart_getc(uint8_t port, uint8_t *ch);

/* DMA TX primitives */
int bsp_uart_dma_tx_start(uint8_t port, const void *mem, uint16_t len);
void bsp_uart_dma_tx_stop(uint8_t port);
int bsp_uart_dma_tx_busy(uint8_t port);

/* DMA RX primitives */

uint16_t bsp_uart_dma_rx_pos(uint8_t port, uint16_t rx_len);
void bsp_uart_clear_idle_flag(uint8_t port);

void bsp_uart_rx_resync(uint8_t port, uint8_t *rx_buf, uint16_t rx_len);
void bsp_uart_rx_suspend(uint8_t port);

/* BSP -> middleware ISR hook
 * BSP 提供 weak 默认空实现
 * middleware/uart.c 中实现同名函数覆盖它
 */
void __attribute__((weak)) bsp_uart_dma_tx_done_isr(uint8_t port);
void __attribute__((weak)) bsp_uart_rx_idle_isr(uint8_t port);

#ifdef __cplusplus
}
#endif

#endif /* BSP_UART_H */
