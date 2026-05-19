# middleware/ch395/CMakeLists.txt

add_library(ch395_lib STATIC)

target_include_directories( ch395_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(ch395_lib PRIVATE
  ch395hw.c
  ch395cmd.c
  ch395.c
)

target_link_libraries(ch395_lib PUBLIC
  mcu_compile
  bsp_lib
  uart_lib
)

