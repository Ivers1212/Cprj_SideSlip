# ${PROJECT_SOURCE_DIR}/bsp/src/common/CMakeLists.txt
# BSP common source files

# -----------------------------------------------------------------------------#
# 创建 BSP 公共源码库
#
# bsp_common 放和具体芯片关系不强的 BSP 实现。
#
# 例如：
#   bsp_delay.c
#
# 如果 bsp_delay.c 内部依赖 SysTick 或 DWT，仍然可以放这里；
# 但如果它直接写 GD32/STM32 寄存器，就应该拆到具体平台目录。

add_library(bsp_common STATIC)

# -----------------------------------------------------------------------------#
# 公共头文件路径
#
# 对外暴露 bsp/include。
# app 和 middleware 只需要 include 这些公共 BSP 头文件。

target_include_directories(bsp_common PUBLIC
  "${PROJECT_SOURCE_DIR}/bsp/include"
)

# -----------------------------------------------------------------------------#
# 公共源码

target_sources(bsp_common PRIVATE
  bsp_delay.c
)

# -----------------------------------------------------------------------------#
# 依赖关系
#
# mcu_common:
#   Cortex-M3 公共编译选项
#
# mcu_target:
#   当前 DEVICE 的芯片宏和 include 路径
#
# start:
#   启动层和 CMSIS/device header

target_link_libraries(bsp_common PUBLIC
  mcu_common
  mcu_target
  start
)

# -----------------------------------------------------------------------------#
