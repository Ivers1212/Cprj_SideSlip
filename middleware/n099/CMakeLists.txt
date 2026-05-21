# ${PROJECT_SOURCE_DIR}/middleware/n099/CMakeLists.txt
# N099 display middleware

# -----------------------------------------------------------------------------#
# 创建 N099 屏幕驱动库
#
# n099 是屏幕驱动层。
# 它负责：
#   1. 屏幕初始化
#   2. 写命令
#   3. 写数据
#   4. 设置显示窗口
#   5. 写像素/刷区域
#
# 它不负责通用绘图算法。
# 通用绘图算法建议放到 middleware/graphics。

add_library(middleware_n099 STATIC)

# -----------------------------------------------------------------------------#
# 头文件路径

target_include_directories(middleware_n099 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------#
# 源文件
#
# 等你创建 n099.c 后再打开。
# 如果现在还没有 n099.c，这个 target 可以先只建空库，
# 但 STATIC 空库在部分 CMake/生成器下不太合适。
#
# 推荐你先创建一个空的 n099.c。

target_sources(middleware_n099 PRIVATE
  n099.c
)

# -----------------------------------------------------------------------------#
# 依赖 BSP
#
# N099 一般会用到：
#   bsp_spi
#   bsp_gpio
#   bsp_delay
#   背光 PWM 等

target_link_libraries(middleware_n099 PUBLIC
  bsp
)

# -----------------------------------------------------------------------------#
