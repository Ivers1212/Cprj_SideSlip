# middleware/ch395/CMakeLists.txt

add_library(start_obj OBJECT)

target_include_directories(start_obj PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_sources(start_obj PRIVATE
  startup_stm32f103xb.s
  stm32f1xx_it.c
  sys_stubs.c
  system_stm32f1xx.c
)

target_link_libraries(start_obj PRIVATE
  mcu_compile
)

