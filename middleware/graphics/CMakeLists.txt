# ------------------------------# ${PROJECT_SOURCE_DIR}/middleware/CMakeLists.txt
# Middleware layer

# -----------------------------------------------------------------------------#
# middleware 层说明
#
# middleware 位于 app 和 bsp 之间。
#
# 它负责：
#   1. 外设芯片驱动封装，例如 CH395、N099
#   2. 通信协议解析，例如 uart_prot
#   3. 通用功能模块，例如后续 graphics
#
# middleware 可以依赖 bsp。
# middleware 不应该直接依赖具体 MCU 寄存器头文件。

# -----------------------------------------------------------------------------#
# 添加 middleware 子模块

add_subdirectory(ch395)
add_subdirectory(uart)
add_subdirectory(n099)

# -----------------------------------------------------------------------------#
# 创建统一 middleware 聚合目标

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  middleware_ch395
  middleware_uart
  middleware_n099
)

# -----------------------------------------------------------------------------#-----------------------------------------------#
