# ${PROJECT_SOURCE_DIR}/cmake/mcu_common.cmake
# Cortex-M3 common compile and link options
#
# 这个文件只管理 Cortex-M3 内核通用的编译、链接规则。
# 不在这里写具体芯片型号，例如 GD32F103 / STM32F103。
# 不在这里写链接脚本路径，例如 xxx_flash.ld。
# 不在这里写启动文件、芯片头文件、BSP 源文件。
#
# 具体芯片差异应该放到：
#   cmake/mcu_gd32f103.cmake
#   cmake/mcu_stm32f103.cmake

# -----------------------------------------------------------------------------#
# 创建公共 MCU 配置目标
#
# mcu_common 是一个 INTERFACE 库：
#   1. 它本身不编译源文件
#   2. 它只向依赖它的目标传播编译选项、链接选项、宏定义
#   3. 最终 executable 链接 mcu_common 后，会自动继承这些规则

add_library(mcu_common INTERFACE)

# -----------------------------------------------------------------------------#
# C 语言标准
#
# c_std_11 表示要求使用 C11 标准。
# 这里用 target_compile_features，而不是手动写 -std=c11，
# 是因为 CMake 会根据编译器自动选择合适的标准参数。

target_compile_features(mcu_common INTERFACE
  c_std_11
)

# -----------------------------------------------------------------------------#
# 公共编译选项
#
# -mcpu=cortex-m3
#   指定目标 CPU 内核为 Cortex-M3。
#   GD32F103 和 STM32F103 都是 Cortex-M3，所以放在公共配置里。
#
# -mthumb
#   使用 Thumb 指令集。
#   Cortex-M 系列只运行 Thumb/Thumb-2 指令，必须指定。
#
# -ffreestanding
#   声明这是 freestanding 环境，不是 hosted 环境。
#   裸机工程没有完整操作系统和标准运行时环境，适合使用该选项。
#
# -fdata-sections
# -ffunction-sections
#   将每个函数和数据对象放到独立 section 中。
#   配合链接阶段的 -Wl,--gc-sections，可以删除未使用的函数和数据。
#
# -fno-common
#   禁止 common symbol。
#   可以更早暴露重复定义的全局变量问题。
#
# Debug:
#   -Og  适合调试的优化等级
#   -g3  生成更完整的调试信息
#
# Release:
#   -O3  偏性能优化
#
# RelWithDebInfo:
#   -O2 -g 兼顾优化和调试信息
#
# MinSizeRel:
#   -Os  偏代码体积优化
#
# -Wall -Wextra
#   打开常用 C 编译警告。
#   这里只对 C 文件生效，避免影响 ASM 文件。

target_compile_options(mcu_common INTERFACE
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

# -----------------------------------------------------------------------------#
# 公共链接选项
#
# -mcpu=cortex-m3
# -mthumb
#   链接阶段也显式声明目标内核和指令集，保持编译/链接一致。
#
# -Wl,--gc-sections
#   把参数 --gc-sections 传给链接器。
#   作用是丢弃未被引用的 section。
#   需要和编译阶段的 -ffunction-sections / -fdata-sections 配合使用。
#
# -Wl,-Map=xxx.map
#   生成 map 文件。
#   map 文件可以用来分析：
#     1. 最终符号地址
#     2. Flash/RAM 占用
#     3. 哪些目标文件被链接进来了
#     4. 某个函数或变量最终放在哪个段
#
# --specs=nano.specs
#   使用 newlib-nano，减小 C 标准库体积。
#
# --specs=nosys.specs
#   提供一组默认的系统调用桩。
#   如果工程里自己实现了 syscalls.c，通常也可以保留。
#   但如果后续出现 _write、_sbrk 等重复定义，再回来调整。

target_link_options(mcu_common INTERFACE
  -mcpu=cortex-m3
  -mthumb
  -Wl,--gc-sections
  -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
  --specs=nano.specs
  --specs=nosys.specs
)

# -----------------------------------------------------------------------------#
# 公共宏定义
#
# Debug 构建时定义 DEBUG=1。
# 非 Debug 构建时定义 NDEBUG=1。
#
# 这些宏可以在代码里用于控制：
#   1. 调试日志
#   2. assert 行为
#   3. 调试辅助代码
#   4. 不同构建类型下的条件编译

target_compile_definitions(mcu_common INTERFACE
  $<$<CONFIG:Debug>:DEBUG=1>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1>
)

# -----------------------------------------------------------------------------#
