# ${PROJECT_SOURCE_DIR}/CMakeLists.txt
# 顶层CMake, 负责整个工程搭建

# -----------------------------------------------------------------------------#
# 工程入口与最终目标

cmake_minimum_required(VERSION 3.20)

project(
  SideSlip
  VERSION 0.10
  LANGUAGES C ASM
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 目标 MCU 平台选择
# 可选值:
#   gd32f103
#   stm32f103
set(DEVICE "gd32f103" CACHE STRING "Target device")
set_property(CACHE DEVICE PROPERTY STRINGS
  gd32f103c8t6
  stm32f103c8t6
)

message(STATUS "Project      : ${PROJECT_NAME}")
message(STATUS "Project dir  : ${PROJECT_SOURCE_DIR}")
message(STATUS "Build dir    : ${PROJECT_BINARY_DIR}")
message(STATUS "DEVICE       : ${DEVICE}")

add_executable(${PROJECT_NAME}
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

# -----------------------------------------------------------------------------#
# 编译与链接配置

# Cortex-M3 公共编译/链接选项
include("${CMAKE_CURRENT_LIST_DIR}/cmake/mcu_common.cmake")

# 根据 DEVICE 选择具体芯片平台配置
if(DEVICE STREQUAL "gd32f103c8t6")
  include("${CMAKE_CURRENT_LIST_DIR}/cmake/mcu_gd32f103.cmake")

elseif(DEVICE STREQUAL "stm32f103c8t6")
  include("${CMAKE_CURRENT_LIST_DIR}/cmake/mcu_stm32f103.cmake")

else()
  message(FATAL_ERROR "Unsupported DEVICE: ${DEVICE}")
endif()

# 先把公共 MCU 规则和目标 MCU 规则挂到最终 elf 上
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    mcu_common
    mcu_target
)

# -----------------------------------------------------------------------------#
# 管理子目录

add_subdirectory(startup)
add_subdirectory(bsp)
add_subdirectory(middleware)
# app 当前只有 main.c，已经直接加入 add_executable
# 后续 app 里业务文件变多后，再启用 add_subdirectory(app)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    startup
    bsp
    middleware
)

# -----------------------------------------------------------------------------#
# 附加功能

include("${CMAKE_CURRENT_LIST_DIR}/cmake/postbuild.cmake")

# 将 compile_commands.json 复制到工程根目录，方便 clangd / nvim 使用
set(_CC_SRC "${PROJECT_BINARY_DIR}/compile_commands.json")
set(_CC_DST "${PROJECT_SOURCE_DIR}/compile_commands.json")

add_custom_target(compile_commands_to_root ALL
  COMMAND ${CMAKE_COMMAND} -E rm -f "${_CC_DST}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_CC_SRC}" "${_CC_DST}"
  COMMENT "Refreshing compile_commands.json in project root"
)

add_dependencies(compile_commands_to_root ${PROJECT_NAME})
