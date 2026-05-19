# cmake/toolchain.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 避免 try_compile 生成可执行文件（裸机环境下容易失败）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 如果你用 scoop/环境变量已经能找到，可不写绝对路径
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# 工具
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE   arm-none-eabi-size)
set(CMAKE_AR     arm-none-eabi-ar)
set(CMAKE_RANLIB arm-none-eabi-ranlib)

# 可选：让 CMake 的 find_* 不去主机系统乱找库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
