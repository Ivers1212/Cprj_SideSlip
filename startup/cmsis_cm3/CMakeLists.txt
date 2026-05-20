# ${PROJECT_SOURCE_DIR}/startup/cmsis_cm3/CMakeLists.txt
# CMSIS Cortex-M3 core headers

# -----------------------------------------------------------------------------#
# CMSIS Core 是头文件库，不产生目标文件。
#
# 这里提供：
#   core_cm3.h
#   cmsis_compiler.h
#   cmsis_gcc.h
#   cmsis_version.h

add_library(cmsis_cm3 INTERFACE)

target_include_directories(cmsis_cm3 INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------#
