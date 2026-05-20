---
id: ${PROJECT_SOURCE_DIR}\readme
aliases: []
tags: []
---
# readme

```cmd
cmake -S . -B build-gd32 -G Ninja `
  -D CMAKE_TOOLCHAIN_FILE=cmake/toolchain_arm-none-eabi-gcc.cmake `
  -D DEVICE=gd32f103 `
  -D CMAKE_BUILD_TYPE=Debug

cmake --build build-gd32
```

```cmd
cmake -S . -B build-stm32 -G Ninja `
-D CMAKE_TOOLCHAIN_FILE=cmake/toolchain_arm-none-eabi-gcc.cmake `
-D DEVICE=stm32f103 `
-D CMAKE_BUILD_TYPE=Debug

cmake --build build-stm32
```

顶层CMake的依赖预期:
后面的文件需要保证生成这些target:

```cmd
cmake/mcu_common.cmake
  -> mcu_common

cmake/mcu_gd32f103.cmake
  -> mcu_target

cmake/mcu_stm32f103.cmake
  -> mcu_target

startup/CMakeLists.txt
  -> start

bsp/CMakeLists.txt
  -> bsp

middleware/CMakeLists.txt
  -> middleware
```
