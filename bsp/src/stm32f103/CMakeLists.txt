# ${PROJECT_SOURCE_DIR}/bsp/src/stm32f103/CMakeLists.txt
# stm32f103 BSP implementation

# -----------------------------------------------------------------------------#
# 创建 stm32f103 BSP 源码库

add_library(bsp_stm32f103 STATIC)

# -----------------------------------------------------------------------------#
# BSP 公共头文件路径

target_include_directories(bsp_stm32f103 PUBLIC
  "${PROJECT_SOURCE_DIR}/bsp/include"
)

# -----------------------------------------------------------------------------#
# STM32 平台 BSP 源码

target_sources(bsp_stm32f103 PRIVATE
  bsp_clock_stm32f103.c
  bsp_gpio_stm32f103.c
  bsp_spi_stm32f103.c
  bsp_uart_stm32f103.c
)

# -----------------------------------------------------------------------------#
# 依赖关系

target_link_libraries(bsp_stm32f103 PUBLIC
  mcu_common
  mcu_target
  start
)

# -----------------------------------------------------------------------------#
