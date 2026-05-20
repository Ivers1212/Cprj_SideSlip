# ${PROJECT_SOURCE_DIR}/startup/stm32f103/CMakeLists.txt
# STM32F103C8T6 startup objects

# -----------------------------------------------------------------------------#
# STM32F103C8T6 启动对象库

add_library(start_obj_stm32f103c8t6 OBJECT)

# -----------------------------------------------------------------------------#
# 当前平台头文件路径
#
# 提供：
#   stm32f103xb.h
#   stm32f1xx.h
#   system_stm32f1xx.h
#   stm32f1xx_it.h

target_include_directories(start_obj_stm32f103c8t6 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------#
# 当前平台启动源文件

target_sources(start_obj_stm32f103c8t6 PRIVATE
  startup_stm32f103xb.s
  system_stm32f1xx.c
  sys_stubs.c
  stm32f1xx_it.c
)

# -----------------------------------------------------------------------------#
# 依赖关系

target_link_libraries(start_obj_stm32f103c8t6 PUBLIC
  cmsis_cm3
  mcu_common
  mcu_target
)

# -----------------------------------------------------------------------------#
