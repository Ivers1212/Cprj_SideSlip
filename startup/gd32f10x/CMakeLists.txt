# ${PROJECT_SOURCE_DIR}/startup/gd32f103/CMakeLists.txt
# GD32F103C8T6 startup objects

# -----------------------------------------------------------------------------#
# GD32F103C8T6 启动对象库
#
# OBJECT 库会编译出 .o 文件，但不会打包成 .a。
# 对启动文件来说，用 OBJECT 库更稳，因为启动向量表、Reset_Handler
# 必须直接进入最终 ELF。

add_library(start_obj_gd32f103c8t6 OBJECT)

# -----------------------------------------------------------------------------#
# 当前平台头文件路径
#
# 提供：
#   gd32f10x.h
#   system_gd32f10x.h
#   gd32f10x_it.h
#   systick.h

target_include_directories(start_obj_gd32f103c8t6 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------#
# 当前平台启动源文件
#
# startup_gd32f10x_md.S:
#   向量表、Reset_Handler、默认中断处理函数
#
# system_gd32f10x.c:
#   SystemInit、SystemCoreClock、系统时钟配置
#
# syscalls.c:
#   newlib 系统调用桩，例如 _write、_sbrk、_read
#
# gd32f10x_it.c:
#   异常/中断处理函数，例如 HardFault_Handler、SysTick_Handler

target_sources(start_obj_gd32f103c8t6 PRIVATE
  startup_gd32f10x_md.S
  system_gd32f10x.c
  syscalls.c
  gd32f10x_it.c
)

# -----------------------------------------------------------------------------#
# 依赖关系
#
# cmsis_cm3:
#   提供 Cortex-M3 CMSIS Core 头文件
#
# mcu_common:
#   提供 Cortex-M3 公共编译/链接选项
#
# mcu_target:
#   提供 GD32F103C8T6 芯片宏、链接脚本、平台 include 路径

target_link_libraries(start_obj_gd32f103c8t6 PUBLIC
  cmsis_cm3
  mcu_common
  mcu_target
)

# -----------------------------------------------------------------------------#
