# ${PROJECT_SOURCE_DIR}/middleware/uart/CMakeLists.txt
# UART middleware

# -----------------------------------------------------------------------------#
# 创建 UART 中间件库
#
# 这里建议放：
#   1. UART 抽象封装
#   2. UART 协议解析
#   3. 环形缓冲
#
# 不建议在这里直接写 gd32f10x.h / stm32f103xb.h。
# 真正的 UART 寄存器配置应该在 bsp_uart_xxx.c 里。

add_library(middleware_uart STATIC)

# -----------------------------------------------------------------------------#
# 头文件路径

target_include_directories(middleware_uart PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------#
# 源文件

target_sources(middleware_uart PRIVATE
  uart.c
  uart_prot.c
)

# -----------------------------------------------------------------------------#
# 依赖 BSP

target_link_libraries(middleware_uart PUBLIC
  bsp
)

# -----------------------------------------------------------------------------#
