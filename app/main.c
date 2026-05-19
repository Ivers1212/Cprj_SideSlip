/* 文件名: main.c
 * 描述: <简要描述该模块的功能和用途>
 *
 * 边界:
 * 1. 该模块实现了 <模块功能描述>。
 * 2. 仅限于在当前模块内部使用的功能通过私有函数和变量进行封装。
 * 3. 对外提供公共API接口供其他模块调用。
 */

/* Private includes
 * **************************************************************/
#include "bsp_delay.h"
#include "ch395.h"
#include "system_stm32f1xx.h"
#include "uart.h"
#include "uart_prot.h"
#include <stdint.h>

/* Private define
 * **************************************************************/
/* 宏定义 - 根据需求设置 */
#define APP_UART_CH395 1U  /* USART2 -> CH395 */
#define APP_UART_SCREEN 2U /* USART3 -> 屏幕控制板 */

#define APP_SOCK_LISTEN 0U
#define APP_SOCK_OFP 1U
#define APP_SOCK_SW 2U

/* Private typedef
 * **************************************************************/
/* 可用于内部的结构体或类型定义 */

/* Private variable
 * **************************************************************/
/* 文件内私有变量，使用 static 限制作用域 */
static ch395_socket_cfg_t g_socks[3];
static ch395_cfg_t g_ch395_cfg;
typedef struct {
  uint8_t local_ip[4];
  uint16_t listen_port;  /* socket0 src_port */
  uint16_t ofp_des_port; /* socket1 src_port */
} mfd_net_cfg_t;
/* Private function prototypes
 * **************************************************************/
/* 声明本模块内部函数，不对外公开 */
static void app_uart_init_all();
static void app_ch395_cfg_init();
static void app_build_uart_frame(const uart_protocol_t *protocol,
                                 uint8_t frame[16]);

/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */
static void app_uart_init_all(void) {
  uart_cfg_t cfg_ch395 = {115200U, 1U};
  uart_cfg_t cfg_screen = {115200U, 1U};

  uart_init(APP_UART_CH395, &cfg_ch395);
  uart_init(APP_UART_SCREEN, &cfg_screen);
}

static void app_ch395_cfg_init(void) {
  memset(&g_ch395_cfg, 0, sizeof(g_ch395_cfg));
  memset(g_socks, 0, sizeof(g_socks));

  g_ch395_cfg.ch395_uart_port = APP_UART_CH395;
  g_ch395_cfg.socket_count = 1;
  g_ch395_cfg.sockets = g_socks;
  g_ch395_cfg.ch395_baudrate = 115200U;

  /* 本地IP/GW/MASK 先填默认值 */
  g_ch395_cfg.local_ip[0] = 192;
  g_ch395_cfg.local_ip[1] = 168;
  g_ch395_cfg.local_ip[2] = 8;
  g_ch395_cfg.local_ip[3] = 136;

  g_ch395_cfg.gw_ip[0] = 192;
  g_ch395_cfg.gw_ip[1] = 168;
  g_ch395_cfg.gw_ip[2] = 8;
  g_ch395_cfg.gw_ip[3] = 1;

  g_ch395_cfg.ip_mask[0] = 255;
  g_ch395_cfg.ip_mask[1] = 255;
  g_ch395_cfg.ip_mask[2] = 255;
  g_ch395_cfg.ip_mask[3] = 0;

  g_ch395_cfg.socket_count = 3;

  g_ch395_cfg.sockets = g_socks;

  g_ch395_cfg.dbg_uart_port = APP_UART_SCREEN;

  /* socket0: 本地监听 */
  g_socks[APP_SOCK_LISTEN].proto_type = PROTO_TYPE_UDP;
  g_socks[APP_SOCK_LISTEN].src_port = 7034; /* 你自己定 */
  g_socks[APP_SOCK_LISTEN].des_ip[0] = 0xFF;
  g_socks[APP_SOCK_LISTEN].des_ip[1] = 0xFF;
  g_socks[APP_SOCK_LISTEN].des_ip[2] = 0xFF;
  g_socks[APP_SOCK_LISTEN].des_ip[3] = 0xFF;

  /* socket1: OFP */
  g_socks[APP_SOCK_OFP].proto_type = PROTO_TYPE_UDP;
  g_socks[APP_SOCK_OFP].src_port = 1001;
  g_socks[APP_SOCK_OFP].des_port = 9217;
  g_socks[APP_SOCK_OFP].des_ip[0] = 192;
  g_socks[APP_SOCK_OFP].des_ip[1] = 168;
  g_socks[APP_SOCK_OFP].des_ip[2] = 8;
  // g_socks[APP_SOCK_OFP].des_ip[3] = 100;
  g_socks[APP_SOCK_OFP].des_ip[3] = 130;

  /* socket2: 软件 */
  g_socks[APP_SOCK_SW].proto_type = PROTO_TYPE_UDP;
  g_socks[APP_SOCK_SW].src_port = 1000;
  g_socks[APP_SOCK_SW].des_port = 7030;
  g_socks[APP_SOCK_SW].des_ip[0] = 192;
  g_socks[APP_SOCK_SW].des_ip[1] = 168;
  g_socks[APP_SOCK_SW].des_ip[2] = 8;
  g_socks[APP_SOCK_SW].des_ip[3] = 134;
  // g_socks[APP_SOCK_SW].des_ip[3] = 134;
}

static uint8_t app_apply_mfd_id(uint8_t mfd_id) {
  mfd_net_cfg_t cfg;

  switch (mfd_id) {
  case 0x00:

    cfg.local_ip[0] = 192;
    cfg.local_ip[1] = 168;
    cfg.local_ip[2] = 8;
    cfg.local_ip[3] = 136;
    cfg.listen_port = 7034;
    cfg.ofp_des_port = 9217;
    break;

  case 0x01:
    cfg.local_ip[0] = 192;
    cfg.local_ip[1] = 168;
    cfg.local_ip[2] = 8;
    cfg.local_ip[3] = 137;
    cfg.listen_port = 7035;
    cfg.ofp_des_port = 9218;
    break;

  case 0x02:
    cfg.local_ip[0] = 192;
    cfg.local_ip[1] = 168;
    cfg.local_ip[2] = 8;
    cfg.local_ip[3] = 138;
    cfg.listen_port = 7036;
    cfg.ofp_des_port = 9219;
    break;

  case 0x03:
    cfg.local_ip[0] = 192;
    cfg.local_ip[1] = 168;
    cfg.local_ip[2] = 8;
    cfg.local_ip[3] = 139;
    cfg.listen_port = 7037;
    cfg.ofp_des_port = 9220;
    break;
  default:
    return 1U;
  }

  memcpy(g_ch395_cfg.local_ip, cfg.local_ip, 4);
  g_socks[APP_SOCK_LISTEN].src_port = cfg.listen_port;
  g_socks[APP_SOCK_OFP].des_port = cfg.ofp_des_port;

  return ch395_init(&g_ch395_cfg);
}

static void app_build_uart_frame(const uart_protocol_t *protocol,
                                 uint8_t frame[16]) {
  memcpy(&frame[0], protocol->header, 2);
  memcpy(&frame[2], protocol->data, 12);
  memcpy(&frame[14], protocol->checksum, 2);
}

/* Hook function definitions
 * **************************************************************/
/* 覆盖hook函数 */
void handle_uart_protocol(uint8_t port, uart_protocol_t *protocol) {
  uint8_t cmd;
  uint8_t value;
  uint8_t frame[16];

  if ((port != APP_UART_SCREEN) || (protocol == 0)) {
    return;
  }

  cmd = protocol->data[1];    /* Byte3 */
  value = protocol->data[11]; /* Byte13 */

  app_build_uart_frame(protocol, frame);

  switch (cmd) {
  case 0x82:
    ch395_udp_sendto(APP_SOCK_OFP, frame, 16, g_socks[1].des_ip,
                     g_socks[1].des_port, 50);
    ch395_udp_sendto(APP_SOCK_SW, frame, 16, g_socks[2].des_ip,
                     g_socks[2].des_port, 50);
    break;

  case 0x83:
    ch395_udp_sendto(APP_SOCK_OFP, frame, 16, g_socks[1].des_ip,
                     g_socks[1].des_port, 50);
    break;

  case 0x84:
    ch395_udp_sendto(APP_SOCK_SW, frame, 16, g_socks[2].des_ip,
                     g_socks[2].des_port, 50);
    break;
  case 0xA0:
    /* 修改mfd_id -> 改本地IP/PORT */
    app_apply_mfd_id(value);
    break;

  default:
    break;
  }
}

void ch395_udp_rx_hook(uint8_t sockindex, const uint8_t *payload, uint16_t len,
                       const uint8_t *src_ip, uint16_t src_port) {
  (void)src_ip;
  (void)src_port;

  if (sockindex != 0U) {
    return;
  }

  uart_write(APP_UART_SCREEN, payload, len);
}

/* User Code
 * **************************************************************/
/* 用户代码 */

int main(void) {
  uart_protocol_t screen_proto = {0};

  /* 时钟 / BSP init */
  SystemInit();
  MySysTick_Config(SystemCoreClock / 8000000); // 1us systick handler

  app_uart_init_all();

  app_ch395_cfg_init();
  ch395_init(&g_ch395_cfg);

  while (1) {
    /* 链路1: 屏幕板 -> UART3 -> 协议解析 */
    uart_protocol_rx_poll(APP_UART_SCREEN, &screen_proto);

    /* 链路2: 上位机UDP -> CH395 -> hook -> UART3 */
    ch395_poll();
  }
}
