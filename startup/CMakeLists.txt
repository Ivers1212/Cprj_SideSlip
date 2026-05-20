# ${PROJECT_SOURCE_DIR}/startup/CMakeLists.txt
# Startup layer dispatcher

# -----------------------------------------------------------------------------#
# 添加 CMSIS Cortex-M3 头文件库

add_subdirectory(cmsis_cm3)

# -----------------------------------------------------------------------------#
# 创建统一启动层目标
#
# start 是一个 INTERFACE 聚合库。
# 顶层 CMakeLists.txt 永远只需要链接 start：
#
#   target_link_libraries(${PROJECT_NAME} PRIVATE start)
#
# 具体使用 GD32 还是 STM32，由 DEVICE 决定。

add_library(start INTERFACE)

# -----------------------------------------------------------------------------#
# 根据 DEVICE 选择启动对象库

if(DEVICE STREQUAL "gd32f103c8t6")

  add_subdirectory(gd32f103)

  target_sources(start INTERFACE
    $<TARGET_OBJECTS:start_obj_gd32f103c8t6>
  )

  target_link_libraries(start INTERFACE
    start_obj_gd32f103c8t6
  )

elseif(DEVICE STREQUAL "stm32f103c8t6")

  add_subdirectory(stm32f103)

  target_sources(start INTERFACE
    $<TARGET_OBJECTS:start_obj_stm32f103c8t6>
  )

  target_link_libraries(start INTERFACE
    start_obj_stm32f103c8t6
  )

else()

  message(FATAL_ERROR "Unsupported DEVICE in startup layer: ${DEVICE}")

endif()

# -----------------------------------------------------------------------------#
