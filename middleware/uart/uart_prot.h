#ifndef UART_PROT__H
#define UART_PROT__H

#include "uart.h"

#ifndef UART_HEADER_SIZE
#define UART_HEADER_SIZE 2
#endif

#ifndef UART_DATA_SIZE
#define UART_DATA_SIZE 12
#endif

#ifndef UART_CHECK_SIZE
#define UART_CHECK_SIZE 2
#endif

typedef enum {
  CHECKSUM_NONE = 0,
  CHECKSUM_ODD,
  CHECKSUM_EVEN,
  CHECKSUM_CUSTOM
} checksum_type_t;

typedef struct {
  uint8_t header[UART_HEADER_SIZE];
  uint8_t data[UART_DATA_SIZE];
  checksum_type_t checksum_type;
  uint8_t checksum[UART_CHECK_SIZE];
} uart_protocol_t;

void uart_protocol_rx_poll(uint8_t port, uart_protocol_t *protocol);

void handle_uart_protocol(uint8_t port, uart_protocol_t *protocol);
void handle_protocol_error(uint8_t port, int error_code);

#endif
