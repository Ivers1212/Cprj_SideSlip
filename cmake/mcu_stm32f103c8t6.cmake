# ${PROJECT_SOURCE_DIR}/cmake/mcu_stm32f103c8t6.cmake
# STM32F103C8T6 target configuration

# -----------------------------------------------------------------------------#
# 创建当前设备配置目标
#
# mcu_target 是一个 INTERFACE 库：
#   1. 它不编译源文件
#   2. 它只传播当前 DEVICE 的芯片宏、链接脚本、设备头文件路径
#   3. 顶层 executable 链接 mcu_target 后，会继承这些配置

add_library(mcu_target INTERFACE)

# -----------------------------------------------------------------------------#
# 当前设备信息

set(MCU_VENDOR "STMicroelectronics" CACHE INTERNAL "MCU vendor")
set(MCU_FAMILY "STM32F103" CACHE INTERNAL "MCU family")
set(MCU_DEVICE "STM32F103C8T6" CACHE INTERNAL "MCU device")
set(MCU_CORE "Cortex-M3" CACHE INTERNAL "MCU core")

message(STATUS "MCU vendor   : ${MCU_VENDOR}")
message(STATUS "MCU family   : ${MCU_FAMILY}")
message(STATUS "MCU device   : ${MCU_DEVICE}")
message(STATUS "MCU core     : ${MCU_CORE}")

# -----------------------------------------------------------------------------#
# 链接脚本
#
# STM32F103C8T6:
#   Flash: 64 KB
#   SRAM : 20 KB

set(MCU_LINKER_SCRIPT
  "${PROJECT_SOURCE_DIR}/ld/stm32f103/STM32F103C8TX_FLASH.ld"
  CACHE INTERNAL "Linker script"
)

message(STATUS "Linker script: ${MCU_LINKER_SCRIPT}")

# -----------------------------------------------------------------------------#
# 当前设备宏定义
#
# STM32F103xB:
#   STM32 CMSIS device header 常用芯片选择宏。
#   对应 STM32F103C8T6 这类 medium-density 64 KB Flash 设备。
#
# HSE_VALUE:
#   外部高速晶振频率。
#   如果你的板子是 8 MHz 晶振，保持 8000000U。
#   如果实际硬件不是 8 MHz，必须修改这里。

target_compile_definitions(mcu_target INTERFACE
  STM32F103xB
  HSE_VALUE=8000000U
)

# -----------------------------------------------------------------------------#
# 当前设备头文件路径
#
# startup/stm32f103:
#   stm32f103xb.h
#   stm32f1xx.h
#   system_stm32f1xx.h
#   stm32f1xx_it.h
#
# startup/cmsis_cm3:
#   core_cm3.h
#   cmsis_compiler.h
#   cmsis_gcc.h
#   cmsis_version.h

target_include_directories(mcu_target INTERFACE
  "${PROJECT_SOURCE_DIR}/startup/stm32f103"
  "${PROJECT_SOURCE_DIR}/startup/cmsis_cm3"
)

# -----------------------------------------------------------------------------#
# 当前设备链接选项

target_link_options(mcu_target INTERFACE
  -T${MCU_LINKER_SCRIPT}
)

# -----------------------------------------------------------------------------#
