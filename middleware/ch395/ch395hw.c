/* 文件名: <module_name>.c
 * 描述: <简要描述该模块的功能和用途>
 *
 * 边界:
 * 1. 该模块实现了 <模块功能描述>。
 * 2. 仅限于在当前模块内部使用的功能通过私有函数和变量进行封装。
 * 3. 对外提供公共API接口供其他模块调用。
 */

/* Private includes
 * **************************************************************/
#include "ch395hw.h"   // 包含与本文件相关的头文件，暴露公共接口
#include "bsp_ch395.h" //
#include "bsp_delay.h"
#include "ch395.h"
#include "stm32f1xx_it.h"
#include "uart.h"
#include <stdbool.h> // 布尔类型
#include <stdint.h>  // 标准整数类型

/* Private define
 * **************************************************************/
/* 宏定义 - 根据需求设置 */

// #define
#define CMD_START_HANDEL()
#define CMD_END_HANDEL()

/* Private typedef
 * **************************************************************/
/* 可用于内部的结构体或类型定义 */

// typedef struct {
//
// } stuct_name;

/* Private variable
 * **************************************************************/
/* 文件内私有变量，使用 static 限制作用域 */

// static uint8_t variable_name; // 数据缓冲区

/* Private function prototypes
 * **************************************************************/
/* 声明本模块内部函数，不对外公开 */

// static void PrivateValname(void);

/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */

/* Public APIs
 * **************************************************************/
/* 对外提供的公共API接口函数 */

void ch395PinsInit(void) { bsp_ch395pins_init(); }
/* Hardware Reset */
/* 注意：
 * CH395 reset 期间会使 UART RX 线维持低电平一段时间，
 * USART 可能将其误判为异常接收帧，导致 RX DMA 首字节污染。
 * 因此在 ch395Reset() 后，必须清空 UART RX DMA/ringbuffer，
 * 或在 reset 期间临时关闭 UART RX。
 */

void ch395Reset(void) {
  bsp_ch395RSTI_reset();
  delay_ms(10);
  bsp_ch395RSTI_set();
}

void xWriteCH395Cmd(uint8_t cmd) {
  CMD_START_HANDEL();
  uart_putc_poll(PORT_NO, SER_SYNC_CODE1);
  uart_putc_poll(PORT_NO, SER_SYNC_CODE2);
  uart_putc_poll(PORT_NO, cmd);
}

void xWriteCH395Data(uint8_t mdata) { uart_putc_poll(PORT_NO, mdata); }

uint8_t xReadCH395Data(void) {
  uint32_t start = tick_us;
  uint8_t data = 0xFE;
  while (uart_rx_available(1) < 1) {
    if ((tick_us - start) > 5000U)
      return 0xFE;
  };
  uart_rx_getc(PORT_NO, &data);
  return data;
}

void xEndCH395Cmd(void) { CMD_END_HANDEL(); }

uint8_t Query395Interrupt(void) { return bsp_readINT(); }

void ch395_BRS(uint32_t baud) { bsp_ch395_BaudrateSet(baud); }

/* Weak hook override
 * **************************************************************/
/* 如果需要，用户可以在其他地方覆盖这些钩子函数 */
__attribute__((weak)) void custom_callback(void) {
  // 默认空实现，用户可以重写
}
