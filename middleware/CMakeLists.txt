# ${PROJECT_SOURCE_DIR}/middleware/CMakeLists.txt
# Middleware layer

# -----------------------------------------------------------------------------#
# middleware 层说明
#
# middleware 位于 app 和 bsp 之间。
#
# 它负责放置和具体业务相关、但尽量不直接依赖芯片寄存器的模块，例如：
#   1. CH395 网络模块驱动/协议封装
#   2. UART 协议解析
#   3. N099 屏幕驱动
#   4. graphics 通用绘图层
#
# middleware 可以依赖 bsp。
# middleware 不应该直接依赖 gd32f10x.h / stm32f103xb.h。
#
# 推荐依赖方向：
#
#   app
#     -> middleware
#        -> bsp
#           -> startup / mcu_target
#
# 不推荐：
#
#   middleware
#     -> gd32f10x.h
#     -> stm32f103xb.h

# -----------------------------------------------------------------------------#
# 添加各个 middleware 子模块

add_subdirectory(ch395)
add_subdirectory(uart)
add_subdirectory(n099)
add_subdirectory(graphics)
add_subdirectory(spi)

# -----------------------------------------------------------------------------#
# 创建统一 Middleware 聚合目标
#
# middleware 是一个 INTERFACE 聚合库。
# 它本身不编译源码，只负责把各个 middleware 子模块聚合起来。
#
# 顶层 CMakeLists.txt 只需要链接 middleware：
#
#   target_link_libraries(${PROJECT_NAME} PRIVATE middleware)
#
# 不需要逐个关心 ch395、uart、n099、graphics。

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  ch395
  uart
  # spi
  # n099
  # graphics
)

# -----------------------------------------------------------------------------#
