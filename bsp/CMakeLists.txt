# ${PROJECT_SOURCE_DIR}/bsp/CMakeLists.txt
# Board Support Package

# -----------------------------------------------------------------------------#
# BSP 层说明
#
# BSP, Board Support Package, 负责板级外设适配。
#
# 它向上提供统一接口，例如：
#   bsp_uart_init()
#   bsp_uart_write()
#   bsp_delay_ms()
#   bsp_spi_write()
#   bsp_gpio_init()
#
# 它向下依赖具体芯片平台，例如：
#   gd32f103
#   stm32f103
#
# app / middleware 不应该直接关心当前使用的是 GD32 还是 STM32。
# 它们只应该依赖 bsp 暴露出来的公共头文件。

# -----------------------------------------------------------------------------#
# 添加 BSP 公共源码库

add_subdirectory(src/common)

# -----------------------------------------------------------------------------#
# 创建统一 BSP 聚合目标
#
# bsp 是一个 INTERFACE 聚合库。
# 它本身不编译源码，只负责把：
#   1. bsp_common
#   2. 当前 DEVICE 对应的平台 BSP 库
# 聚合起来，提供给 app / middleware 使用。

add_library(bsp INTERFACE)

# -----------------------------------------------------------------------------#
# 根据 DEVICE 选择具体平台 BSP 实现

if(DEVICE STREQUAL "gd32f103")

  add_subdirectory(src/gd32f103)

  target_link_libraries(bsp INTERFACE
    bsp_common
    bsp_gd32f103c8t6
  )

elseif(DEVICE STREQUAL "stm32f103")

  add_subdirectory(src/stm32f103)

  target_link_libraries(bsp INTERFACE
    bsp_common
    bsp_stm32f103c8t6
  )

else()

  message(FATAL_ERROR "Unsupported DEVICE in BSP layer: ${DEVICE}")

endif()

# -----------------------------------------------------------------------------#
