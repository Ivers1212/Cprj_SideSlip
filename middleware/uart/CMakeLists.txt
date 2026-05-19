# middleware/uart/CMakeLists.txt

add_library(uart_lib STATIC)

target_include_directories(uart_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(uart_lib PRIVATE
  uart.c
  uart_prot.c
)

target_link_libraries(uart_lib PUBLIC
  mcu_compile
  bsp_lib
)

