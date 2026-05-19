# mcu_link.cmake

#--- 添加接口库 ---

add_library(mcu_link INTERFACE)

#--- 定义变量脚本Path ---

set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/ld/STM32F103C8TX_FLASH.ld")

#---  ---

target_link_options(mcu_link INTERFACE
  -mcpu=cortex-m3
  -mthumb
  "-T${LINKER_SCRIPT}"
  -nostartfiles
  -nostdlib
  -Wl,--gc-sections
  "-Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map"
)
