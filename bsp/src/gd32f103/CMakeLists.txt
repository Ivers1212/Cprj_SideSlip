# ${PROJECT_SOURCE_DIR}/bsp/src/gd32f103/CMakeLists.txt
# gd32f103 BSP implementation

# -----------------------------------------------------------------------------#
# 创建 gd32f103 BSP 源码库
#
# 这个库只放 gd32f103 平台相关实现。
#
# 这里可以直接 include:
#   gd32f10x.h
#
# 但对外头文件仍然应该是 bsp/include 下的统一接口。

add_library(bsp_gd32f103 STATIC)

# -----------------------------------------------------------------------------#
# BSP 公共头文件路径

target_include_directories(bsp_gd32f103 PUBLIC
  "${PROJECT_SOURCE_DIR}/bsp/include"
)

# -----------------------------------------------------------------------------#
# GD32 平台 BSP 源码

target_sources(bsp_gd32f103 PRIVATE
  bsp_clock_gd32f103.c
  bsp_gpio_gd32f103.c
  bsp_spi_gd32f103.c
  bsp_uart_gd32f103.c
)

# -----------------------------------------------------------------------------#
# 依赖关系
#
# mcu_target 会提供：
#   GD32F10X_MD
#   HXTAL_VALUE
#   startup/gd32f103 include path
#   startup/cmsis_cm3 include path
#
# start 会提供：
#   CMSIS/device header 相关依赖
#
# bsp_common 暂时不在这里链接，统一由上层 bsp 聚合。

target_link_libraries(bsp_gd32f103 PUBLIC
  mcu_common
  mcu_target
  start
)

# -----------------------------------------------------------------------------#
