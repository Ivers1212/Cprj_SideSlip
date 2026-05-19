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

#include "ch395.h"
#include "CH395CMD.h"
#include "bsp_ch395.h"
#include "bsp_delay.h"
#include "ch395cmd.h"
#include "ch395hw.h"
#include "stm32f1xx_it.h"
#include "system_stm32f1xx.h"
#include "uart.h"
#include <stdint.h>

/* Private define
 * **************************************************************/
/* 宏定义 - 根据需求设置 */

/* Private typedef
 * **************************************************************/
/* 可用于内部的结构体或类型定义 */

typedef struct {
  uint8_t rx_buf[2048];
  uint16_t rx_len;
  uint8_t int_status; // 初始化时要把int_status 配置为 SINT_STAT_SENDBUF_FREE
} ch395_socket_ctx_t;

/* Private variable
 * **************************************************************/
/* 文件内私有变量，使用 static 限制作用域 */

static struct _CH395_SYS s_ch395_inf;  /* Save the CH395 information */
static struct _SOCK_INF s_sock_inf[8]; /* Save the socket information */
static ch395_socket_ctx_t s_sock_ctx[8];

static uint8_t s_socket_count = 0;
static uint8_t s_ch395_uart_port = 1;
static uint8_t s_dbg_uart_port = 2;

/* Private function prototypes
 * **************************************************************/
/* 声明本模块内部函数，不对外公开 */
static void ch395_prepare_param(const ch395_cfg_t *cfg);
static uint8_t ch395_hw_startup(const ch395_cfg_t *cfg);
static uint8_t ch395_wait_link_up(uint32_t timeout_ms);
static uint8_t ch395_socket_open(uint8_t sockindex);
static void ch395_handle_global_interrupt(void);
static void ch395_try_recv_udp(uint8_t sockindex);
static void ch395_handle_udp_echo(uint8_t socketindex);
static uint8_t ch395_wait_sendbuf_free(uint8_t sockindex, uint32_t timeout_ms);

/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */

static void ch395_prepare_param(const ch395_cfg_t *cfg) {
  memset(&s_ch395_inf, 0,
         sizeof(s_ch395_inf)); /* Clear all s_ch395_inf to zero */
  memset(&s_sock_inf, 0,
         sizeof(s_sock_inf)); /* Clear all s_ch395_inf to zero */
  memset(&s_sock_ctx, 0,
         sizeof(s_sock_ctx)); /* Clear all s_ch395_inf to zero */

  s_socket_count = cfg->socket_count;
  if (s_socket_count > 8U) {
    s_socket_count = 8U;
  }

  s_dbg_uart_port = cfg->dbg_uart_port;
  s_ch395_uart_port = cfg->ch395_uart_port;

  memcpy(
      s_ch395_inf.IPAddr, cfg->local_ip,
      sizeof(s_ch395_inf
                 .IPAddr)); /* Enter the IP address in the s_ch395_inf file */
  memcpy(s_ch395_inf.GWIPAddr, cfg->gw_ip,
         sizeof(s_ch395_inf.GWIPAddr)); /* Enter the gateway IP address in the
                                  s_ch395_inf file */
  memcpy(s_ch395_inf.MASKAddr, cfg->ip_mask,
         sizeof(s_ch395_inf.MASKAddr)); /* Enter the mask IP address in the
                                    s_ch395_inf file */
  for (uint8_t i = 0; i < s_socket_count; i++) {
    memcpy(s_sock_inf[i].IPAddr, cfg->sockets[i].des_ip,
           sizeof(s_sock_inf[i].IPAddr));

    s_sock_inf[i].DesPort = cfg->sockets[i].des_port;
    s_sock_inf[i].SourPort = cfg->sockets[i].src_port;
    s_sock_inf[i].ProtoType = cfg->sockets[i].proto_type;
  }
}

static uint8_t ch395_hw_startup(const ch395_cfg_t *cfg) {
  uint8_t ver;
  uint8_t check;

  ch395PinsInit();
  ch395_BRS(cfg->ch395_baudrate);
  delay_ms(20);

  uart_rx_suspend(s_ch395_uart_port);
  ch395Reset();
  delay_ms(100);
  uart_rx_resume(s_ch395_uart_port);
  delay_ms(2);

  ver = CH395CMDGetVer();
  my_printf(s_dbg_uart_port, "ver = %x\r\n", ver);

  check = CH395CMDCheckExist(0x65);
  if (check != 0x9A) {
    my_printf(s_dbg_uart_port, "check fail = %x\r\n", check);
    return CH395_ERR_UNKNOW;
  }

  my_printf(s_dbg_uart_port, "check = %x\r\n", check);
  return CMD_ERR_SUCCESS;
}

static uint8_t ch395_wait_link_up(uint32_t timeout_ms) {
  uint32_t start_ms;
  uint32_t now_ms;
  uint8_t phy_status;

  start_ms = tick_us / 1000U;

  while (1) {
    phy_status = CH395CMDGetPHYStatus();
    if (phy_status != PHY_DISCONN) {
      s_ch395_inf.PHYStat = phy_status;
      my_printf(s_dbg_uart_port, "ethernet connected\r\n");
      my_printf(s_dbg_uart_port, "phy = %x\r\n", phy_status);
      return CMD_ERR_SUCCESS;
    }

    if (timeout_ms != 0U) {
      now_ms = tick_us / 1000U;
      if ((uint32_t)(now_ms - start_ms) >= timeout_ms) {
        my_printf(s_dbg_uart_port, "wait link timeout\r\n");
        return CH395_ERR_UNKNOW;
      }
    }

    delay_ms(200);
  }
}

static uint8_t ch395_socket_open(uint8_t sockindex) {
  if (sockindex >= s_socket_count) {
    return CH395_ERR_UNKNOW;
  }

  CH395CMDSetSocketDesIP(sockindex, s_sock_inf[sockindex].IPAddr);
  CH395CMDSetSocketProtType(sockindex, s_sock_inf[sockindex].ProtoType);
  CH395CMDSetSocketDesPort(sockindex, s_sock_inf[sockindex].DesPort);
  CH395CMDSetSocketSourPort(sockindex, s_sock_inf[sockindex].SourPort);

  s_sock_ctx[sockindex].rx_len = 0;
  s_sock_ctx[sockindex].int_status = SINT_STAT_SENDBUF_FREE;

  return CH395CMDOpenSocket(sockindex);
}

static void ch395_handle_global_interrupt(void) {
  uint16_t glob_int;
  uint8_t sockindex;
  uint8_t sock_int;

  glob_int = CH395CMDGetGlobIntStatus();

  if (glob_int & GINT_STAT_UNREACH) {
    my_printf(s_dbg_uart_port, "GINT_STAT_UNREACH\r\n");
  }

  if (glob_int & GINT_STAT_IP_CONFLI) {
    my_printf(s_dbg_uart_port, "GINT_STAT_IP_CONFLI\r\n");
  }

  if (glob_int & GINT_STAT_PHY_CHANGE) {
    my_printf(s_dbg_uart_port, "GINT_STAT_PHY_CHANGE\r\n");
  }

  if (glob_int & GINT_STAT_DHCP) {
    my_printf(s_dbg_uart_port, "GINT_STAT_DHCP\r\n");
  }

  for (sockindex = 0; sockindex < s_socket_count; sockindex++) {
    if (glob_int & ((uint16_t)GINT_STAT_SOCK0 << sockindex)) {
      sock_int = CH395CMDGetSocketInt(sockindex);
      s_sock_ctx[sockindex].int_status |= sock_int;

      if (s_sock_ctx[sockindex].int_status & SINT_STAT_SEND_OK) {
        s_sock_ctx[sockindex].int_status &= (uint8_t)(~SINT_STAT_SEND_OK);
      }

      if (s_sock_ctx[sockindex].int_status & SINT_STAT_CONNECT) {
        s_sock_ctx[sockindex].int_status &= (uint8_t)(~SINT_STAT_CONNECT);
        my_printf(s_dbg_uart_port, "sock%d CONNECT\r\n", sockindex);
      }

      if (s_sock_ctx[sockindex].int_status & SINT_STAT_DISCONNECT) {
        s_sock_ctx[sockindex].int_status &= (uint8_t)(~SINT_STAT_DISCONNECT);
        my_printf(s_dbg_uart_port, "sock%d DISCONNECT\r\n", sockindex);
      }

      if (s_sock_ctx[sockindex].int_status & SINT_STAT_TIM_OUT) {
        s_sock_ctx[sockindex].int_status &= (uint8_t)(~SINT_STAT_TIM_OUT);
        my_printf(s_dbg_uart_port, "sock%d TIMEOUT\r\n", sockindex);
      }
    }
  }
}

static void ch395_try_recv_udp(uint8_t sockindex) {

  ch395_socket_ctx_t *ctx;
  uint16_t recv_len;
  uint16_t src_port;

  if (sockindex >= s_socket_count) {
    return;
  }

  ctx = &s_sock_ctx[sockindex];

  if ((ctx->int_status & SINT_STAT_RECV) == 0U) {
    return;
  }

  ctx->int_status &= (uint8_t)(~SINT_STAT_RECV);

  recv_len = CH395CMDGetRecvLength(sockindex);
  if (recv_len == 0U) {
    return;
  }

  if (recv_len > sizeof(ctx->rx_buf)) {
    ctx->rx_len = 0U;
    return;
  }

  ctx->rx_len = recv_len;
  CH395CMDGetRecvData(sockindex, ctx->rx_len, ctx->rx_buf);

  /* UDP server 模式：前8字节是来源信息 */
  if (ctx->rx_len < 8U) {
    ctx->rx_len = 0U;
    return;
  }

  src_port = (uint16_t)ctx->rx_buf[2] | ((uint16_t)ctx->rx_buf[3] << 8);

  ch395_udp_rx_hook(sockindex, &ctx->rx_buf[8], (uint16_t)(ctx->rx_len - 8U),
                    &ctx->rx_buf[4], src_port);

  ctx->rx_len = 0U;
}

static void ch395_handle_udp_echo(uint8_t sockindex) {
  ch395_socket_ctx_t *ctx;
  uint16_t port;
  uint16_t i;

  if (sockindex >= s_socket_count) {
    return;
  }

  ctx = &s_sock_ctx[sockindex];

  if (ctx->rx_len == 0U) {
    if (ctx->int_status & SINT_STAT_RECV) {
      ctx->int_status &= (uint8_t)(~SINT_STAT_RECV);

      ctx->rx_len = CH395CMDGetRecvLength(sockindex);
      my_printf(s_dbg_uart_port, "recv len = %d\r\n", ctx->rx_len);

      if (ctx->rx_len == 0U) {
        return;
      }

      if (ctx->rx_len > sizeof(ctx->rx_buf)) {
        my_printf(s_dbg_uart_port, "recv too long = %d\r\n", ctx->rx_len);
        ctx->rx_len = 0U;
        return;
      }

      CH395CMDGetRecvData(sockindex, ctx->rx_len, ctx->rx_buf);

      if (ctx->rx_len < 8U) {
        my_printf(s_dbg_uart_port, "packet too short\r\n");
        ctx->rx_len = 0U;
        return;
      }

      port = (uint16_t)ctx->rx_buf[2] | ((uint16_t)ctx->rx_buf[3] << 8);

      my_printf(s_dbg_uart_port, "src port = %d\r\n", port);
      my_printf(s_dbg_uart_port, "src ip = %d.%d.%d.%d\r\n", ctx->rx_buf[4],
                ctx->rx_buf[5], ctx->rx_buf[6], ctx->rx_buf[7]);

      my_printf(s_dbg_uart_port, "payload = ");
      for (i = 8U; i < ctx->rx_len; i++) {
        uart_write(s_dbg_uart_port, &ctx->rx_buf[i], 1);
      }
      my_printf(s_dbg_uart_port, "\r\n");
    }
  } else {
    if (ctx->int_status & SINT_STAT_SENDBUF_FREE) {
      ctx->int_status &= (uint8_t)(~SINT_STAT_SENDBUF_FREE);

      port = (uint16_t)ctx->rx_buf[2] | ((uint16_t)ctx->rx_buf[3] << 8);

      CH395UDPSendTo(&ctx->rx_buf[8], (uint32_t)(ctx->rx_len - 8U),
                     &ctx->rx_buf[4], port, sockindex);

      my_printf(s_dbg_uart_port, "echo back done\r\n");
      ctx->rx_len = 0U;
    }
  }
}

static uint8_t ch395_wait_sendbuf_free(uint8_t sockindex, uint32_t timeout_ms) {
  uint32_t start_ms;
  uint32_t now_ms;

  if (sockindex >= s_socket_count) {
    return CH395_ERR_UNKNOW;
  }

  start_ms = tick_us / 1000U;

  while (1) {
    if (s_sock_ctx[sockindex].int_status & SINT_STAT_SENDBUF_FREE) {
      return CMD_ERR_SUCCESS;
    }

    if (Query395Interrupt() == 0U) {
      ch395_handle_global_interrupt();
    }

    if (timeout_ms != 0U) {
      now_ms = tick_us / 1000U;
      if ((uint32_t)(now_ms - start_ms) >= timeout_ms) {
        return CH395_ERR_UNKNOW;
      }
    }
  }
}
/* Public Code
 * **************************************************************/
/* 对外提供的公共API接口函数 */

uint8_t ch395_init(const ch395_cfg_t *cfg) {
  uint8_t ret;
  uint8_t i;

  if (cfg == NULL) {
    return CH395_ERR_UNKNOW;
  }

  if ((cfg->socket_count > 0U) && (cfg->sockets == NULL)) {
    return CH395_ERR_UNKNOW;
  }

  ch395_prepare_param(cfg);

  ret = ch395_hw_startup(cfg);
  if (ret != CMD_ERR_SUCCESS) {
    return ret;
  }

  CH395CMDSetIPAddr(s_ch395_inf.IPAddr);
  CH395CMDSetGWIPAddr(s_ch395_inf.GWIPAddr);
  CH395CMDSetMASKAddr(s_ch395_inf.MASKAddr);

  ret = CH395CMDInitCH395();
  my_printf(s_dbg_uart_port, "init = %x\r\n", ret);
  if (ret != CMD_ERR_SUCCESS) {
    return ret;
  }

  ret = ch395_wait_link_up(5000U);
  if (ret != CMD_ERR_SUCCESS) {
    return ret;
  }

  for (i = 0; i < s_socket_count; i++) {
    ret = ch395_socket_open(i);
    if (ret != CMD_ERR_SUCCESS) {
      my_printf(s_dbg_uart_port, "open socket %d fail = %x\r\n", i, ret);
      return ret;
    }
  }

  CH395CMDEnablePing(1);
  return CMD_ERR_SUCCESS;
}

void ch395_poll(void) {
  uint8_t i;

  if (Query395Interrupt() == 0U) {
    ch395_handle_global_interrupt();
  }

  for (i = 0; i < s_socket_count; i++) {
    if (s_sock_inf[i].ProtoType == PROTO_TYPE_UDP) {
      ch395_try_recv_udp(i);
    }
  }
}

uint8_t ch395_udp_sendto_nowait(uint8_t sockindex, const uint8_t *buf,
                                uint16_t len, const uint8_t *ip,
                                uint16_t port) {
  if (sockindex >= s_socket_count) {
    return CH395_ERR_UNKNOW;
  }

  if ((buf == 0) || (ip == 0) || (len == 0U)) {
    return CH395_ERR_UNKNOW;
  }

  if (s_sock_inf[sockindex].ProtoType != PROTO_TYPE_UDP) {
    return CH395_ERR_UNKNOW;
  }

  CH395CMDSetSocketDesIP(sockindex, (uint8_t *)ip);
  CH395CMDSetSocketDesPort(sockindex, port);
  CH395CMDSendData(sockindex, (uint8_t *)buf, len);

  return CMD_ERR_SUCCESS;
}

uint8_t ch395_udp_sendto(uint8_t sockindex, const uint8_t *buf, uint16_t len,
                         const uint8_t *ip, uint16_t port,
                         uint32_t timeout_ms) {
  uint8_t ret;

  ret = ch395_wait_sendbuf_free(sockindex, timeout_ms);
  if (ret != CMD_ERR_SUCCESS) {
    return ret;
  }

  s_sock_ctx[sockindex].int_status &= (uint8_t)(~SINT_STAT_SENDBUF_FREE);

  return ch395_udp_sendto_nowait(sockindex, buf, len, ip, port);
}

/* Weak hook override
 * **************************************************************/
/* 如果需要，用户可以在其他地方覆盖这些钩子函数 */
__attribute__((weak)) void
ch395_udp_rx_hook(uint8_t sockindex, const uint8_t *payload, uint16_t len,
                  const uint8_t *src_ip, uint16_t src_port) {
  // 默认空实现，用户可以重写
  (void)sockindex;
  (void)payload;
  (void)len;
  (void)src_ip;
  (void)src_port;
}
