# mcu_common.cmake
add_library(mcu_compile INTERFACE)

target_compile_features(mcu_compile INTERFACE c_std_11)

target_compile_options(mcu_compile INTERFACE
  -mcpu=cortex-m3
  -mthumb
  -ffreestanding
  -fdata-sections
  -ffunction-sections
  -fno-common

  $<$<CONFIG:Debug>:-Og -g3>
  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:RelWithDebInfo>:-O2 -g>
  $<$<CONFIG:MinSizeRel>:-Os>

  $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra>
)

target_compile_definitions(mcu_compile INTERFACE
  STM32F103xB
  $<$<CONFIG:Debug>:DEBUG=1>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1>
)

target_include_directories(mcu_compile INTERFACE
  "${PROJECT_SOURCE_DIR}/start"
  "${PROJECT_SOURCE_DIR}/start/inc"
)

