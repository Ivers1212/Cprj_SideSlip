/* 文件名: bsp_ch395_stm32f103.c
 * 描述: <简要描述该模块的功能和用途>
 *
 * 边界:
 * 1. 该模块实现了 <模块功能描述>。
 * 2. 仅限于在当前模块内部使用的功能通过私有函数和变量进行封装。
 * 3. 对外提供公共API接口供其他模块调用。
 */

/* Private includes
 * **************************************************************/
#include "bsp_ch395.h" // 包含与本文件相关的头文件，暴露公共接口
#include "stm32f1xx.h"
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
static void prv_gpio_enable(GPIO_TypeDef *BANK);
/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */

static void prv_gpio_enable(GPIO_TypeDef *BANK) {
  if (BANK == GPIOA) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // 设置 GPIOA 时钟
  } else if (BANK == GPIOB) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // 设置 GPIOA 时钟
  }
}
/* Public APIs
 * **************************************************************/
/* 对外提供的公共API接口函数 */

/* ch395pins_init
 * 绑定MCU的引脚资源给CH395的
 * RST, RSTI, INT#, SEL
 */

void bsp_ch395pins_init() {
  prv_gpio_enable(GPIOA);

  /* RST - PA8 - INPUTPULLUP*/
  GPIOA->CRH &= ~GPIO_CRH_MODE8; // 清除 PA8 的模式
  GPIOA->CRH &= ~GPIO_CRH_CNF8;  // 清除 PA8 的配置
  GPIOA->ODR &= ~GPIO_ODR_ODR8;  // 清除 PA8 ODR
  GPIOA->CRH |= GPIO_CRH_CNF8_1; // 设置为输入
  GPIOA->ODR |= GPIO_ODR_ODR8;   // 设置为上拉

  /* SCK - PA5 - OUTPUT OPENDRAIN*/
  GPIOA->CRL &= ~GPIO_CRL_MODE5; // 清除 PA5 的模式
  GPIOA->CRL &= ~GPIO_CRL_CNF5;  // 清除 PA5 的配置
  GPIOA->CRL |= GPIO_CRL_MODE5;  // 设置为输出模式（最大速度 50 MHz）
  GPIOA->CRL |= GPIO_CRL_CNF5_0; // 设置为开漏输出（Open Drain）
  // GPIOA->CRL &= ~GPIO_CRL_CNF5; // 设置为推挽输出（Push-Pull）

  /* SDO - PA6 - OUTPUT OPENDRAIN*/
  GPIOA->CRL &= ~GPIO_CRL_MODE6; // 清除 PA6 的模式
  GPIOA->CRL &= ~GPIO_CRL_CNF6;  // 清除 PA6 的配置
  GPIOA->CRL |= GPIO_CRL_MODE6;  // 设置为输出模式（最大速度 50 MHz）
  GPIOA->CRL |= GPIO_CRL_CNF6_0; // 设置为开漏输出（Open Drain）

  /* SDI - PA7 - OUTPUT OPENDRAIN*/
  GPIOA->CRL &= ~GPIO_CRL_MODE7; // 清除 PA7 的模式
  GPIOA->CRL &= ~GPIO_CRL_CNF7;  // 清除 PA7 的配置
  GPIOA->CRL |= GPIO_CRL_MODE7;  // 设置为输出模式（最大速度 50 MHz）
  GPIOA->CRL |= GPIO_CRL_CNF7_1; // 设置为开漏输出（Open Drain）

  /* RSTI - PA11 - OUTPUT OPENDRAIN*/
  GPIOA->CRH &= ~GPIO_CRH_MODE11; // 清除 PA11 的模式
  GPIOA->CRH &= ~GPIO_CRH_CNF11;  // 清除 PA11 的配置
  GPIOA->CRH |= GPIO_CRH_MODE11;  // 设置为输出模式（最大速度 50 MHz）
  // GPIOA->CRH |= GPIO_CRH_CNF11_1; // 设置为开漏输出（Open Drain）
  GPIOA->CRL &= ~GPIO_CRL_CNF5;  // 设置为推挽输出（Push-Pull）
  GPIOA->BSRR |= GPIO_BSRR_BS11; // 设置 PA11 输出高电平

  /* INT# - PA9 - INPUTPULLUP*/
  GPIOA->CRH &= ~GPIO_CRH_MODE9; // 清除 PA9 的模式
  GPIOA->CRH &= ~GPIO_CRH_CNF9;  // 清除 PA9 的配置
  GPIOA->ODR &= ~GPIO_ODR_ODR9;  // 清除 PA9 ODR
  GPIOA->CRH |= GPIO_CRH_CNF9_1; // 设置为输入
  GPIOA->ODR |= GPIO_ODR_ODR9;   // 设置为上拉
}

void bsp_ch395RSTI_set() {
  // 将 RSTI 引脚 (PA11) 设置为高电平（复位解除）
  GPIOA->BSRR |= GPIO_BSRR_BS11; // 设置 PA11 输出高电平
}

void bsp_ch395RSTI_reset() {
  // 将 RSTI 引脚 (PA11) 设置为低电平（复位激活）
  GPIOA->BSRR |= GPIO_BSRR_BR11; // 设置 PA11 输出低电平
}

uint8_t bsp_readINT() {
  // 读取 GPIOA 的输入数据寄存器的 PA9 引脚值
  // PA9 的位置在 GPIOA_IDR 寄存器的第 9 位
  return (GPIOA->IDR & GPIO_IDR_IDR9) >> 9; // PA9 的电平（0 或 1）
}

void bsp_ch395_BaudrateSet(uint32_t baud) {
  if (baud == 115200) {
    GPIOA->BSRR |= GPIO_BSRR_BS5; // 设置 PA5 - SCK 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BS6; // 设置 PA6 - SDO 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BR7; // 设置 PA7 - SDI 输出低电平
  } else if (baud == 9600) {
    GPIOA->BSRR |= GPIO_BSRR_BS5; // 设置 PA5 - SCK 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BS6; // 设置 PA6 - SDO 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BS7; // 设置 PA7 - SDI 输出高电平
  } else {
    GPIOA->BSRR |= GPIO_BSRR_BS5; // 设置 PA5 - SCK 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BS6; // 设置 PA6 - SDO 输出高电平
    GPIOA->BSRR |= GPIO_BSRR_BS7; // 设置 PA7 - SDI 输出高电平
  }
}
/* Weak hook override
 * **************************************************************/
/* 如果需要，用户可以在其他地方覆盖这些钩子函数 */
