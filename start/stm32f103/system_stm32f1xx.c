/**
 ******************************************************************************
 * @file    system_stm32f1xx.c
 * @author  MCD Application Team
 * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer System Source File.
 *
 * 1.  This file provides two functions and one global variable to be called
 *from user application:
 *      - SystemInit(): Setups the system clock (System clock source, PLL
 *Multiplier factors, AHB/APBx prescalers and Flash settings). This function is
 *called at startup just after reset and before branch to main program. This
 *call is made inside the "startup_stm32f1xx_xx.s" file.
 *
 *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be
 *used by the user application to setup the SysTick timer or configure other
 *parameters.
 *
 *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
 *                                 be called whenever the core clock is changed
 *                                 during program execution.
 *
 * 2. After each device reset the HSI (8 MHz) is used as system clock source.
 *    Then SystemInit() function is called, in "startup_stm32f1xx_xx.s" file, to
 *    configure the system clock before to branch to main program.
 *
 * 4. The default value of HSE crystal is set to 8 MHz (or 25 MHz, depending on
 *    the product used), refer to "HSE_VALUE".
 *    When HSE is used as system clock source, directly or through PLL, and you
 *    are using different crystal you have to adapt the HSE value to your own
 *    configuration.
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/** @addtogroup CMSIS
 * @{
 */

/** @addtogroup stm32f1xx_system
 * @{
 */

/** @addtogroup STM32F1xx_System_Private_Includes
 * @{
 */

#include "system_stm32f1xx.h"
#include "stm32f103xb.h"

/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_Defines
 * @{
 */
#define RCC_STARTUP_TIMEOUT (0x5000U)
#if !defined(HSE_VALUE)
#define HSE_VALUE                                                              \
  8000000U /*!< Default value of the External oscillator in Hz.                \
                This value can be provided and adapted by the user             \
              application. */
#endif     /* HSE_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE                                                              \
  8000000U /*!< Default value of the Internal oscillator in Hz.                \
                This value can be provided and adapted by the user             \
              application. */
#endif     /* HSI_VALUE */

/*!< Uncomment the following line if you need to use external SRAM  */
#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) ||    \
    defined(STM32F103xE) || defined(STM32F103xG)
/* #define DATA_IN_ExtSRAM */
#endif /* STM32F100xE || STM32F101xE || STM32F101xG || STM32F103xE ||          \
          STM32F103xG */

/* Note: Following vector table addresses must be defined in line with linker
         configuration. */
/*!< Uncomment the following line if you need to relocate the vector table
     anywhere in Flash or Sram, else the vector table is kept at the automatic
     remap of boot address selected */
/* #define USER_VECT_TAB_ADDRESS */

#if defined(USER_VECT_TAB_ADDRESS)
/*!< Uncomment the following line if you need to relocate your vector Table
     in Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */
#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS                                                  \
  SRAM_BASE /*!< Vector Table base address field.                              \
                 This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET                                                        \
  0x00000000U /*!< Vector Table base offset field.                             \
                   This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS                                                  \
  FLASH_BASE /*!< Vector Table base address field.                             \
                  This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET                                                        \
  0x00000000U /*!< Vector Table base offset field.                             \
                   This value must be a multiple of 0x200. */
#endif        /* VECT_TAB_SRAM */
#endif        /* USER_VECT_TAB_ADDRESS */

/******************************************************************************/

/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_Variables
 * @{
 */

/* This variable is updated in three ways:
    1) by calling CMSIS function SystemCoreClockUpdate()
    2) by calling HAL API function HAL_RCC_GetHCLKFreq()
    3) each time HAL_RCC_ClockConfig() is called to configure the system clock
   frequency Note: If you use this function to configure the system clock; then
   there is no need to call the 2 first functions listed above, since
   SystemCoreClock variable is updated automatically.
*/
uint32_t SystemCoreClock = 8000000;
const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8U] = {0, 0, 0, 0, 1, 2, 3, 4};

/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_FunctionPrototypes
 * @{
 */
static void SetSysClock_72MHz_HSE(void);
static void SetSysClock_HSI8(void);
/**
 * @}
 */

/** @addtogroup STM32F1xx_System_Private_Functions
 * @{
 */

/**
 * @brief  Setup the microcontroller system
 *         Initialize the Embedded Flash Interface, the PLL and update the
 *         SystemCoreClock variable.
 * @note   This function should be used only after reset.
 * @param  None
 * @retval None
 */
void SystemInit(void) {
  /* 1) reset RCC to default -------------------------------------------*/
  // Enable HSI to reset RCC (more safe)
  RCC->CR |= RCC_CR_HSION;
  // Reset CR->CFGR
  RCC->CFGR = 0x00000000U; // U - unsigned, 0x means hex
  // Clear HSE On, PLL On bit
  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_PLLON);
  // Clear External high-speed clock passby, Clock sercurity system bit
  RCC->CR &= ~(RCC_CR_HSEBYP | RCC_CR_CSSON);
  // Clear PLL entry clock source, PLL HSE divder for PLL entry
  // PLL multiplication factor  bits
  RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
  // Clear CSS PLL, HSE, HSI, LSE, LSI's ready interrupt  bits
  RCC->CIR = (RCC_CIR_CSSC | RCC_CIR_PLLRDYC | RCC_CIR_HSERDYC |
              RCC_CIR_HSIRDYC | RCC_CIR_LSERDYC | RCC_CIR_LSIRDYC);
  /* 2) configure Flash wait sates + prefetch --------------------------*/
  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
  /* 3) configure HSE/PLL + AHB/APB prescale + switch SYSCLK -----------*/
  SetSysClock_72MHz_HSE();
  // Update SystemCoreClock
  SystemCoreClockUpdate();
  /* Configure the Vector Table location -------------------------------------*/
#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS |
              VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#endif                         /* USER_VECT_TAB_ADDRESS */
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 *         The SystemCoreClock variable contains the core clock (HCLK), it can
 *         be used by the user application to setup the SysTick timer or
 * configure other parameters.
 *
 * @note   Each time the core clock (HCLK) changes, this function must be called
 *         to update SystemCoreClock variable value. Otherwise, any
 * configuration based on this variable will be incorrect.
 *
 * @note   - The system frequency computed by this function is not the real
 *           frequency in the chip. It is calculated based on the predefined
 *           constant and the selected clock source:
 *
 *           - If SYSCLK source is HSI, SystemCoreClock will contain the
 * HSI_VALUE(*)
 *
 *           - If SYSCLK source is HSE, SystemCoreClock will contain the
 * HSE_VALUE(**)
 *
 *           - If SYSCLK source is PLL, SystemCoreClock will contain the
 * HSE_VALUE(**) or HSI_VALUE(*) multiplied by the PLL factors.
 *
 *         (*) HSI_VALUE is a constant defined in stm32f1xx.h file (default
 * value 8 MHz) but the real value may vary depending on the variations in
 * voltage and temperature.
 *
 *         (**) HSE_VALUE is a constant defined in stm32f1xx.h file (default
 * value 8 MHz or 25 MHz, depending on the product used), user has to ensure
 *              that HSE_VALUE is same as the real frequency of the crystal
 * used. Otherwise, this function may have wrong result.
 *
 *         - The result of this function could be not correct when using
 * fractional value for HSE crystal.
 * @param  None
 * @retval None
 */
void SystemCoreClockUpdate(void) {
  uint32_t sysclk = 0U;

  /* Read SYSCLK source
   * -------------------------------------------------------*/
  uint32_t tmp =
      RCC->CFGR &
      RCC_CFGR_SWS; // 3&2 bit of RCC_CFGR(Clock Configuration Register)

  switch (tmp) {
  case RCC_CFGR_SWS_HSI: /* HSI used as system clock */
    sysclk = HSI_VALUE;
    break;
  case RCC_CFGR_SWS_HSE: /* HSE used as system clock */
    sysclk = HSE_VALUE;
    break;
  case RCC_CFGR_SWS_PLL: /* PLL used as system clock */
  {
    /* Get PLL clock source and multiplication factor ----------------------*/
    uint32_t pllmull =
        RCC->CFGR & RCC_CFGR_PLLMULL; // PLLMUL[3:0] 21-18 bits of RCC_CFGR
    uint32_t pllsource = RCC->CFGR & RCC_CFGR_PLLSRC; // PLLSER 16bit
    pllmull = (pllmull >> 18U) + 2U;                  // 0000(0)+2->2...

    if (pllsource == 0x00U) {
      /* HSI oscillator clock divided by 2 selected as PLL input clock */
      sysclk =
          (HSI_VALUE >> 1U) * pllmull; // shift logical right is diveded by 2
    } else {
      /* HSE selected as PLL clock entry */
      if (RCC->CFGR &
          RCC_CFGR_PLLXTPRE) { /* HSE oscillator clock divided by 2 */
        sysclk = (HSE_VALUE >> 1U) * pllmull;
      } else {
        sysclk = HSE_VALUE * pllmull;
      }
    }
  } break;

  default:
    sysclk = HSI_VALUE;
    break;
  }

  /* Apply AHB prescaler to get HCLK (= SystemCoreClock) ----------------*/
  uint32_t shift = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >>
                                  4U)]; /* HPRE is 7-4, after shifting logical
            right 4 bit , transfer to actual divisor.*/
  /*const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0,
                                      1, 2, 3, 4, 6, 7, 8, 9};*/
  /*SYSCLK -> AHB prescaler -> HCLK(CPU/DMA/SRAM)*/
  SystemCoreClock = sysclk >> shift;
}

/**
 * @brief System Clock's actual configutation
 * @param  None
 * @retval None
 * @note HSE=8MHz, PLL x9 => SYSCLK = 72MHz
 * AHB = 72MHz, APB1 = 36MHz, APB2 = 72MHZ
 * @note configure RCC_CR & RCC_CFGR
 */
static void SetSysClock_72MHz_HSE() {
  uint32_t timeout = 0U;
  /* 1. Open HSE ---------------------------------------------*/
  RCC->CR |= RCC_CR_HSEON;
  while (((RCC->CR & RCC_CR_HSERDY) == 0U) && (timeout < RCC_STARTUP_TIMEOUT)) {
    timeout++;
  }
  if ((RCC->CR & RCC_CR_HSERDY) == 0U) {
    SetSysClock_HSI8();
    return;
  }

  /* 2. APB1&2's prescale ------------------------------------*/
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

  /* 3. Other Clocks' prescale -------------------------------*/
  RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

  /* 4. Configurate PLL --------------------------------------*/
  RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
  RCC->CFGR |= RCC_CFGR_PLLSRC;
  RCC->CFGR |= RCC_CFGR_PLLMULL9;

  RCC->CR |= RCC_CR_PLLON;
  /* 5. Switch sysclk to PLLCLK ------------------------------*/
  timeout = 0U;

  while (((RCC->CR & RCC_CR_PLLRDY) == 0U) && (timeout < RCC_STARTUP_TIMEOUT)) {
    timeout++;
  }
  if ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
    SetSysClock_HSI8();
    return;
  }

  RCC->CFGR &= ~RCC_CFGR_SW;
  RCC->CFGR |= RCC_CFGR_SW_PLL;

  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
  }
}

/**
 * @brief System Clock's actual configutation
 * @param  None
 * @retval None
 * @note HSE=8MHz, PLL x9 => SYSCLK = 72MHz
 * AHB = 72MHz, APB1 = 36MHz, APB2 = 72MHZ
 */
static void SetSysClock_HSI8() {
  /* 1. ensure about HSI on and ready --------------------------------*/
  RCC->CR |= RCC_CR_HSION;
  while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {
  }

  /* 2. switch sysclk to HSI -----------------------------------------*/
  RCC->CFGR &= ~RCC_CFGR_SW;
  RCC->CFGR |= RCC_CFGR_SW_HSI;

  /* 3 disable PLL, clear PLL configutation -------------------------*/
  RCC->CR &= ~RCC_CR_PLLON;
  while (RCC->CR & RCC_CR_PLLRDY) {
  };
  RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);

  /* 4. clear Other Prescalers ---------------------------------------*/
  RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

  RCC->CFGR &= ~RCC_CFGR_ADCPRE;

  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
  }
}
/**
 * @}
 */

uint32_t MySysTick_Config(uint32_t ticks) {
  if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk) {
    return (1UL); /* Reload value impossible */
  }

  SysTick->LOAD = (uint32_t)(ticks - 1UL); /* set reload register */
  NVIC_SetPriority(SysTick_IRQn,
                   (1UL << __NVIC_PRIO_BITS) -
                       1UL); /* set Priority for Systick Interrupt */
  SysTick->VAL = 0UL;        /* Load the SysTick Counter Value */
  SysTick->CTRL =
      (0 << SysTick_CTRL_CLKSOURCE_Pos) | SysTick_CTRL_TICKINT_Msk |
      SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
  return (0UL);                /* Function successful */
}
/**
 * @}
 */

/**
 * @}
 */
