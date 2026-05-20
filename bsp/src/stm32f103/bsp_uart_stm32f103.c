/* 文件名: bsp_uart.c
 * 描述:
 *  板级 UART + DMA 资源绑定与初始化 (裸核)
 *  负责把具体芯片外设资源(USARTx, DMAx_ChannelY, IRQ, GPIO, CLK)
 *  配置成可用状态，并向上层 UART driver 提供统一硬件原语
 *
 * 边界:
 *  - 本文件只做 hardware bring-up / primitive
 *  - 不实现 ringbuffer / 续发策略 / 协议解析
 *  - middleware 只依赖本文件暴露的函数，不直接碰寄存器资源表
 */

/* Private include
 * **************************************************************/
#include "bsp_uart.h"
#include "stm32f103xb.h"

/* -----------------------------------------------------------------------------*/
/* Private typedef
 * **************************************************************/

typedef struct {
  USART_TypeDef *uart;
  DMA_Channel_TypeDef *tx_dma;
  DMA_Channel_TypeDef *rx_dma;
  IRQn_Type uart_irq;
  IRQn_Type tx_dma_irq;
  IRQn_Type rx_dma_irq;
} bsp_uart_hw_t;

/* -----------------------------------------------------------------------------*/
/* Private define
 * **************************************************************/

#define BSP_UART_DEFAULT_BAUD (115200U)
#define PCLK2_HZ (72000000U)
#define PCLK1_HZ (36000000U)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* -----------------------------------------------------------------------------*/
/* Private variable
 * **************************************************************/

static bsp_uart_hw_t s_uart_hw[] = {
    /* UART1: USART1 + DMA1_Channel4(TX) + DMA1_Channel5(RX) */
    {USART1, DMA1_Channel4, DMA1_Channel5, USART1_IRQn, DMA1_Channel4_IRQn,
     DMA1_Channel5_IRQn},

    /* UART2: USART2 + DMA1_Channel7(TX) + DMA1_Channel6(RX) */
    [1] = {USART2, DMA1_Channel7, DMA1_Channel6, USART2_IRQn,
           DMA1_Channel7_IRQn, DMA1_Channel6_IRQn},

    /* UART3: USART3 + DMA1_Channel2(TX) + DMA1_Channel3(RX) */
    [2] = {USART3, DMA1_Channel2, DMA1_Channel3, USART3_IRQn,
           DMA1_Channel2_IRQn, DMA1_Channel3_IRQn},
};

/* -----------------------------------------------------------------------------*/
/* Private function prototypes
 * **************************************************************/

static uint16_t prv_uart_brr_from_pclk(uint32_t pclk_hz, uint32_t baud);
static void prv_uart1_gpio_init(void);
static void prv_uart2_gpio_init(void);
static void prv_uart3_gpio_init(void);
static void prv_uart_clk_enable(USART_TypeDef *uart);
static void prv_dma_clk_enable(void);
static void prv_dma_clear_flags(DMA_Channel_TypeDef *ch);
static int prv_uart_idle_flag_is_set(uint8_t port);
static void prv_uart_irq_enable(uint8_t port);

/* -----------------------------------------------------------------------------*/
/* Private user code
 * **************************************************************/

/* F1 / oversampling by 16
 * BRR[15:4] = mantissa
 * BRR[3:0]  = fraction
 */
static uint16_t prv_uart_brr_from_pclk(uint32_t pclk_hz, uint32_t baud) {
  uint32_t scaled;
  uint32_t mantissa;
  uint32_t fraction;

  if (baud == 0U) {
    baud = BSP_UART_DEFAULT_BAUD;
  }

  /* scaled ~= 16 * USARTDIV */
  scaled = (pclk_hz + (baud / 2U)) / baud;
  mantissa = scaled / 16U;
  fraction = scaled % 16U;

  if (fraction > 15U) {
    fraction = 15U;
  }

  return (uint16_t)((mantissa << 4) | (fraction & 0x0FU));
}

/* USART1: PA9(TX) AF PP 50MHz, PA10(RX) floating input */
static void prv_uart1_gpio_init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

  /* PA9 */
  GPIOA->CRH &= ~(0xFU << (4U * (9U - 8U)));
  GPIOA->CRH |= (0xBU << (4U * (9U - 8U)));

  /* PA10 */
  GPIOA->CRH &= ~(0xFU << (4U * (10U - 8U)));
  GPIOA->CRH |= (0x4U << (4U * (10U - 8U)));
}

/* USART2: PA2(TX) AF PP 50MHz, PA3(RX) floating input */
static void prv_uart2_gpio_init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

  /* PA2 */
  GPIOA->CRL &= ~(0xFU << (4U * 2U));
  GPIOA->CRL |= (0xBU << (4U * 2U));

  /* PA3 */
  GPIOA->CRL &= ~(0xFU << (4U * 3U));
  GPIOA->CRL |= (0x4U << (4U * 3U));
}

/* USART3: PB10(TX) AF PP 50MHz, PB11(RX) floating input */
static void prv_uart3_gpio_init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

  /* PB10 */
  GPIOB->CRH &= ~(0xFU << (4U * (10U - 8U)));
  GPIOB->CRH |= (0xBU << (4U * (10U - 8U)));

  /* PB11 */
  GPIOB->CRH &= ~(0xFU << (4U * (11U - 8U)));
  GPIOB->CRH |= (0x4U << (4U * (11U - 8U)));
}

static void prv_uart_clk_enable(USART_TypeDef *uart) {
  if (uart == USART1) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  } else if (uart == USART2) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
  } else if (uart == USART3) {
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
  } else {
    /* invalid / unsupported */
  }
}

static void prv_dma_clk_enable(void) { RCC->AHBENR |= RCC_AHBENR_DMA1EN; }

/* 清 channel 的 group flags（TC/HT/TE/GI） */
static void prv_dma_clear_flags(DMA_Channel_TypeDef *ch) {
  if (ch == DMA1_Channel1) {
    DMA1->IFCR = DMA_IFCR_CGIF1;
  } else if (ch == DMA1_Channel2) {
    DMA1->IFCR = DMA_IFCR_CGIF2;
  } else if (ch == DMA1_Channel3) {
    DMA1->IFCR = DMA_IFCR_CGIF3;
  } else if (ch == DMA1_Channel4) {
    DMA1->IFCR = DMA_IFCR_CGIF4;
  } else if (ch == DMA1_Channel5) {
    DMA1->IFCR = DMA_IFCR_CGIF5;
  } else if (ch == DMA1_Channel6) {
    DMA1->IFCR = DMA_IFCR_CGIF6;
  } else if (ch == DMA1_Channel7) {
    DMA1->IFCR = DMA_IFCR_CGIF7;
  } else {
    /* do nothing */
  }
}

static int prv_uart_idle_flag_is_set(uint8_t port) {
  USART_TypeDef *U;

  if (port >= ARRAY_SIZE(s_uart_hw))
    return 0;

  U = s_uart_hw[port].uart;
  return ((U->SR & USART_SR_IDLE) != 0U) ? 1 : 0;
}

static void prv_uart_irq_enable(uint8_t port) {
  if (port >= ARRAY_SIZE(s_uart_hw))
    return;

  NVIC_EnableIRQ(s_uart_hw[port].uart_irq);
}
/* -----------------------------------------------------------------------------*/
/* Public BSP APIs
 * **************************************************************/

void bsp_uart_init(uint8_t port, uint32_t baudrate) {
  bsp_uart_hw_t *hw;
  USART_TypeDef *U;
  uint32_t pclk;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  if (baudrate == 0U) {
    baudrate = BSP_UART_DEFAULT_BAUD;
  }

  hw = &s_uart_hw[port];
  U = hw->uart;

  /* 1) enable peripheral clock + GPIO */
  prv_uart_clk_enable(U);

  if (U == USART1) {
    prv_uart1_gpio_init();
  } else if (U == USART2) {
    prv_uart2_gpio_init();
  } else if (U == USART3) {
    prv_uart3_gpio_init();
  } else {
    return;
  }

  /* 2) disable UE before reconfig */
  U->CR1 &= ~USART_CR1_UE;

  /* 3) 8N1, no parity, no flow control */
  U->CR1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS);
  U->CR2 &= ~(USART_CR2_STOP);
  U->CR3 &= ~(USART_CR3_CTSE | USART_CR3_RTSE);

  /* 4) baud */
  pclk = (U == USART1) ? PCLK2_HZ : PCLK1_HZ;
  U->BRR = prv_uart_brr_from_pclk(pclk, baudrate);

  /* 5) enable TX/RX + UE */
  U->CR1 |= USART_CR1_TE | USART_CR1_RE;
  U->CR1 |= USART_CR1_UE;

  /* 6) clear status by SR then DR read */
  (void)U->SR;
  (void)U->DR;
}

int bsp_uart_putc(uint8_t port, uint8_t ch) {
  USART_TypeDef *U;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return -1;
  }

  U = s_uart_hw[port].uart;

  while ((U->SR & USART_SR_TXE) == 0U) {
    ;
  }

  U->DR = ch;
  return 0;
}

int bsp_uart_getc(uint8_t port, uint8_t *ch) {
  USART_TypeDef *U;
  uint32_t sr;
  uint8_t d;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return -1;
  }

  U = s_uart_hw[port].uart;

  if ((U->SR & USART_SR_RXNE) == 0U) {
    return -1;
  }

  sr = U->SR;
  d = (uint8_t)(U->DR & 0xFFU);
  (void)sr;

  if (ch != 0) {
    *ch = d;
  }

  return 0;
}

void bsp_uart_dma_init(uint8_t port, uint8_t *rx_buf, uint16_t rx_len) {
  bsp_uart_hw_t *hw;
  USART_TypeDef *U;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  hw = &s_uart_hw[port];
  U = hw->uart;

  prv_dma_clk_enable();

  /* ---------------- TX DMA base config ----------------
   * memory -> peripheral
   * CMAR/CNDTR 由 middleware 每次发送时填写
   */
  hw->tx_dma->CCR &= ~DMA_CCR_EN;
  prv_dma_clear_flags(hw->tx_dma);

  hw->tx_dma->CCR = 0U;
  hw->tx_dma->CPAR = (uint32_t)&U->DR;

  hw->tx_dma->CCR |= DMA_CCR_MINC; /* memory increment */
  hw->tx_dma->CCR |= DMA_CCR_DIR;  /* mem -> periph */
  hw->tx_dma->CCR |= DMA_CCR_TCIE; /* transfer complete irq */
  /* 可选：
   * hw->tx_dma->CCR |= DMA_CCR_TEIE;
   */

  /* enable USART DMA TX request */
  U->CR3 |= USART_CR3_DMAT;

  NVIC_EnableIRQ(hw->tx_dma_irq);

  /* ---------------- RX DMA base config ----------------
   * peripheral -> memory(DIR = 0)
   * circular
   * coordinate with USART IDLE
   */
  if ((rx_buf != 0) && (rx_len != 0U)) {
    hw->rx_dma->CCR &= ~DMA_CCR_EN;
    prv_dma_clear_flags(hw->rx_dma);

    hw->rx_dma->CCR = 0U;
    hw->rx_dma->CPAR = (uint32_t)&U->DR;
    hw->rx_dma->CMAR = (uint32_t)rx_buf;
    hw->rx_dma->CNDTR = rx_len;

    hw->rx_dma->CCR |= DMA_CCR_MINC; /* memory increment */
    hw->rx_dma->CCR |= DMA_CCR_CIRC; /* circular mode */
    /* DIR = 0: peripheral -> memory */

    /* 可选：
     * hw->rx_dma->CCR |= DMA_CCR_TEIE;
     * hw->rx_dma->CCR |= DMA_CCR_HTIE;
     * hw->rx_dma->CCR |= DMA_CCR_TCIE;
     */

    /* clear UART status before enabling IDLE path */
    (void)U->SR;
    (void)U->DR;

    /* enable USART RX DMA request + IDLE interrupt */
    U->CR3 |= USART_CR3_DMAR;
    U->CR1 |= USART_CR1_IDLEIE;

    prv_dma_clear_flags(hw->rx_dma);
    hw->rx_dma->CCR |= DMA_CCR_EN;

    /* RX event is handled in USART IRQ, not DMA IRQ */
    prv_uart_irq_enable(port);
  }
}

/* -----------------------------------------------------------------------------*/
/* Tx primitive
 * **************************************************************/

int bsp_uart_dma_tx_start(uint8_t port, const void *mem, uint16_t len) {
  bsp_uart_hw_t *hw;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return -1;
  }

  if ((mem == 0) || (len == 0U)) {
    return -1;
  }

  hw = &s_uart_hw[port];

  /* DMA channel still busy */
  if ((hw->tx_dma->CCR & DMA_CCR_EN) != 0U) {
    return -2;
  }

  /* disable -> clear flags -> fill registers -> enable */
  hw->tx_dma->CCR &= ~DMA_CCR_EN;
  prv_dma_clear_flags(hw->tx_dma);

  hw->tx_dma->CMAR = (uint32_t)mem;
  hw->tx_dma->CNDTR = len;

  hw->tx_dma->CCR |= DMA_CCR_EN;
  return 0;
}

void bsp_uart_dma_tx_stop(uint8_t port) {
  bsp_uart_hw_t *hw;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  hw = &s_uart_hw[port];

  hw->tx_dma->CCR &= ~DMA_CCR_EN;
  prv_dma_clear_flags(hw->tx_dma);
}

int bsp_uart_dma_tx_busy(uint8_t port) {
  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return 0;
  }

  return ((s_uart_hw[port].tx_dma->CCR & DMA_CCR_EN) != 0U) ? 1 : 0;
}

/* -----------------------------------------------------------------------------*/
/* Rx primitive
 * **************************************************************/

uint16_t bsp_uart_dma_rx_pos(uint8_t port, uint16_t rx_len) {
  bsp_uart_hw_t *hw;
  uint16_t ndtr;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return 0U;
  }

  if (rx_len == 0U) {
    return 0U;
  }

  hw = &s_uart_hw[port];
  ndtr = (uint16_t)hw->rx_dma->CNDTR;

  if (ndtr > rx_len) {
    return 0U;
  }

  return (uint16_t)(rx_len - ndtr);
}

void bsp_uart_clear_idle_flag(uint8_t port) {
  USART_TypeDef *U;
  volatile uint32_t tmp;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  U = s_uart_hw[port].uart;

  tmp = U->SR;
  tmp = U->DR;
  (void)tmp;
}

/* -----------------------------------------------------------------------------*/
/* IRQ Handlers
 * **************************************************************/

/* UART1 TX DMA: DMA1_Channel4 */
void DMA1_Channel4_IRQHandler(void) {
  /* clear DMA1 channel4 group flags */
  DMA1->IFCR = DMA_IFCR_CGIF4;

  /* stop channel */
  DMA1_Channel4->CCR &= ~DMA_CCR_EN;

  /* notify middleware: port 0 */
  bsp_uart_dma_tx_done_isr(0U);
}

/* UART1 RX IDLE: USART1 */
void USART1_IRQHandler(void) {
  if (prv_uart_idle_flag_is_set(0U) != 0) {
    bsp_uart_clear_idle_flag(0U);
    bsp_uart_rx_idle_isr(0U);
  }

  /* 这里先不处理 ORE/FE/NE/PE
   * 后续如果需要，可以统一在这里补错误统计/恢复
   */
}

/* UART2 TX DMA: DMA1_Channel4 */
void DMA1_Channel7_IRQHandler(void) {
  /* clear DMA1 channel7 group flags */
  DMA1->IFCR = DMA_IFCR_CGIF7;

  /* stop channel */
  DMA1_Channel7->CCR &= ~DMA_CCR_EN;

  /* notify middleware: port 1 */
  bsp_uart_dma_tx_done_isr(1U);
}

/* UART2 RX IDLE: USART2 */
void USART2_IRQHandler(void) {
  if (prv_uart_idle_flag_is_set(1U) != 0) {
    bsp_uart_clear_idle_flag(1U);
    bsp_uart_rx_idle_isr(1U);
  }

  /* 这里先不处理 ORE/FE/NE/PE
   * 后续如果需要，可以统一在这里补错误统计/恢复
   */
}

/* UART3 TX DMA: DMA1_Channel4 */
void DMA1_Channel2_IRQHandler(void) {
  /* clear DMA1 channel4 group flags */
  DMA1->IFCR = DMA_IFCR_CGIF2;

  /* stop channel */
  DMA1_Channel2->CCR &= ~DMA_CCR_EN;

  /* notify middleware: port 2 */
  bsp_uart_dma_tx_done_isr(2U);
}

/* UART3 RX IDLE: USART3 */
void USART3_IRQHandler(void) {
  if (prv_uart_idle_flag_is_set(2U) != 0) {
    bsp_uart_clear_idle_flag(2U);
    bsp_uart_rx_idle_isr(2U);
  }

  /* 这里先不处理 ORE/FE/NE/PE
   * 后续如果需要，可以统一在这里补错误统计/恢复
   */
}

void bsp_uart_rx_resync(uint8_t port, uint8_t *rx_buf, uint16_t rx_len) {
  bsp_uart_hw_t *hw;
  USART_TypeDef *U;
  volatile uint32_t tmp;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  if ((rx_buf == 0U) || (rx_len == 0U)) {
    return;
  }

  hw = &s_uart_hw[port];
  U = hw->uart;

  /* 1) stop UART RX DMA request first */
  U->CR3 &= ~USART_CR3_DMAR;

  /* 2) 可选：先关 IDLEIE，避免重配期间进中断 */
  U->CR1 &= ~USART_CR1_IDLEIE;

  /* 3) stop RX DMA channel */
  hw->rx_dma->CCR &= ~DMA_CCR_EN;

  /* 4) clear DMA flags */
  prv_dma_clear_flags(hw->rx_dma);

  /* 5) reload DMA registers */
  hw->rx_dma->CPAR = (uint32_t)&U->DR;
  hw->rx_dma->CMAR = (uint32_t)rx_buf;
  hw->rx_dma->CNDTR = rx_len;

  /* 6) clear UART status/data residue */
  tmp = U->SR;
  tmp = U->DR;
  (void)tmp;

  /* 7) clear DMA flags again for safety */
  prv_dma_clear_flags(hw->rx_dma);

  /* 8) enable RX DMA channel */
  hw->rx_dma->CCR |= DMA_CCR_EN;

  /* 9) re-enable UART RX DMA request */
  U->CR3 |= USART_CR3_DMAR;

  /* 10) re-enable IDLE interrupt */
  U->CR1 |= USART_CR1_IDLEIE;
}

void bsp_uart_rx_suspend(uint8_t port) {
  bsp_uart_hw_t *hw;
  USART_TypeDef *U;
  volatile uint32_t tmp;

  if (port >= ARRAY_SIZE(s_uart_hw)) {
    return;
  }

  hw = &s_uart_hw[port];
  U = hw->uart;

  /* 1) 关 IDLE 中断，避免 suspend 期间进 ISR */
  U->CR1 &= ~USART_CR1_IDLEIE;

  /* 2) 关 UART 的 DMA RX 请求 */
  U->CR3 &= ~USART_CR3_DMAR;

  /* 3) 关 RX DMA 通道 */
  hw->rx_dma->CCR &= ~DMA_CCR_EN;

  /* 4) 清 DMA flag */
  prv_dma_clear_flags(hw->rx_dma);

  /* 5) 清 UART 残留状态 */
  tmp = U->SR;
  tmp = U->DR;
  (void)tmp;
}

// uart isr
/* weak hook, overridden by middleware/uart.c */
void __attribute__((weak)) bsp_uart_dma_tx_done_isr(uint8_t port) {
  (void)port;
}
/* weak hook, overridden by middleware/uart.c */
void __attribute__((weak)) bsp_uart_rx_idle_isr(uint8_t port) { (void)port; }
