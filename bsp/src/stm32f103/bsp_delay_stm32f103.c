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
#include "bsp_delay.h" // 包含与本文件相关的头文件，暴露公共接口
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"
#include <stdbool.h> // 布尔类型
#include <stdint.h>  // 标准整数类型

/* Private define
 * **************************************************************/
/* 宏定义 - 根据需求设置 */

/* Private typedef
 * **************************************************************/
/* 可用于内部的结构体或类型定义 */

/* Private variable
 * **************************************************************/
/* 文件内私有变量，使用 static 限制作用域 */

/* Private function prototypes
 * **************************************************************/
/* 声明本模块内部函数，不对外公开 */

/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */

/* Public APIs
 * **************************************************************/
/* 对外提供的公共API接口函数 */

void delay_us(uint8_t us) {
  uint32_t start = tick_us;
  uint32_t current;
  do {
    current = tick_us;
  } while ((current - start) < us);
}

void delay_ms(uint16_t ms) {
  uint32_t start = tick_us / 1000;
  uint32_t current;
  do {
    current = tick_us / 1000;
  } while ((current - start) < ms);
}

/* Weak hook override
 * **************************************************************/
/* 如果需要，用户可以在其他地方覆盖这些钩子函数 */
