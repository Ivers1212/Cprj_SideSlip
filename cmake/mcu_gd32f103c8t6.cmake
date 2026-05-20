# ${PROJECT_SOURCE_DIR}/cmake/mcu_gd32f103c8t6.cmake
# GD32F103C8T6 target configuration

# -----------------------------------------------------------------------------#
# 创建当前设备配置目标
#
# mcu_target 是一个 INTERFACE 库：
#   1. 它不编译源文件
#   2. 它只传播当前 DEVICE 的芯片宏、链接脚本、设备头文件路径
#   3. 顶层 executable 链接 mcu_target 后，会继承这些配置
#
# 注意：
#   mcu_target 这个名字在不同 DEVICE 下保持一致。
#   顶层 CMakeLists.txt 不需要关心当前到底是 GD32 还是 STM32。

add_library(mcu_target INTERFACE)

# -----------------------------------------------------------------------------#
# 当前设备信息
#
# 这些变量主要用于日志、调试和后续扩展。
# CACHE INTERNAL 表示它们是内部变量，不希望用户在 cmake-gui 或命令行里随便改。

set(MCU_VENDOR "GigaDevice" CACHE INTERNAL "MCU vendor")
set(MCU_FAMILY "GD32F103" CACHE INTERNAL "MCU family")
set(MCU_DEVICE "GD32F103C8T6" CACHE INTERNAL "MCU device")
set(MCU_CORE "Cortex-M3" CACHE INTERNAL "MCU core")

message(STATUS "MCU vendor   : ${MCU_VENDOR}")
message(STATUS "MCU family   : ${MCU_FAMILY}")
message(STATUS "MCU device   : ${MCU_DEVICE}")
message(STATUS "MCU core     : ${MCU_CORE}")

# -----------------------------------------------------------------------------#
# 链接脚本
#
# GD32F103C8T6:
#   Flash: 64 KB
#   SRAM : 20 KB
#
# 这里不在 mcu_common.cmake 里指定链接脚本，
# 因为链接脚本和具体芯片容量强相关。

set(MCU_LINKER_SCRIPT
  "${PROJECT_SOURCE_DIR}/ld/gd32f103/gd32f10x_flash.ld"
  CACHE INTERNAL "Linker script"
)

message(STATUS "Linker script: ${MCU_LINKER_SCRIPT}")

# -----------------------------------------------------------------------------#
# 当前设备宏定义
#
# GD32F10X_MD:
#   表示 GD32F10x Medium Density 设备。
#   GD32F103C8T6 通常归入 MD 系列。
#
# HXTAL_VALUE:
#   外部高速晶振频率。
#   如果你的板子是 8 MHz 晶振，保持 8000000U。
#   如果实际硬件不是 8 MHz，必须修改这里，否则 SystemCoreClock、串口波特率、
#   SysTick、定时器都会受影响。

target_compile_definitions(mcu_target INTERFACE
  GD32F10X_MD
  HXTAL_VALUE=8000000U
)

# -----------------------------------------------------------------------------#
# 当前设备头文件路径
#
# startup/gd32f103:
#   gd32f10x.h
#   system_gd32f10x.h
#   gd32f10x_it.h
#
# startup/cmsis_cm3:
#   core_cm3.h
#   cmsis_compiler.h
#   cmsis_gcc.h
#   cmsis_version.h
#
# 注意：
#   这里只暴露当前平台需要的 include 路径。
#   不要同时把 startup/gd32f103 和 startup/stm32f103 都暴露出来。

target_include_directories(mcu_target INTERFACE
  "${PROJECT_SOURCE_DIR}/startup/gd32f103"
  "${PROJECT_SOURCE_DIR}/startup/cmsis_cm3"
)

# -----------------------------------------------------------------------------#
# 当前设备链接选项
#
# -Txxx.ld:
#   指定链接脚本。
#
# 链接脚本必须放在 mcu_target 里，而不是 mcu_common 里。
# 因为不同芯片 Flash/RAM 容量和段布局可能不同。

target_link_options(mcu_target INTERFACE
  -T${MCU_LINKER_SCRIPT}
)

# -----------------------------------------------------------------------------#
