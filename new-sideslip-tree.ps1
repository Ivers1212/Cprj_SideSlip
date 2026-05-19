# new-sideslip-tree.ps1
# 一键创建 SideSlip 工程基础目录结构

param(
    [string]$ProjectName = ".",
    [string]$RootPath = "."
)

$ProjectRoot = Join-Path $RootPath $ProjectName

Write-Host "Creating project tree: $ProjectRoot" -ForegroundColor Cyan

# 目录列表
$Dirs = @(
    $ProjectRoot,

    "$ProjectRoot\ld",
    "$ProjectRoot\ld\stm32f103",
    "$ProjectRoot\ld\gd32f103",

    "$ProjectRoot\cmake",

    "$ProjectRoot\startup",
    "$ProjectRoot\startup\stm32f103",
    "$ProjectRoot\startup\gd32f103",

    "$ProjectRoot\bsp",
    "$ProjectRoot\bsp\include",
    "$ProjectRoot\bsp\src",
    "$ProjectRoot\bsp\src\common",
    "$ProjectRoot\bsp\src\stm32f103",
    "$ProjectRoot\bsp\src\gd32f103",

    "$ProjectRoot\middleware",
    "$ProjectRoot\middleware\uart",
    "$ProjectRoot\middleware\ch395",

    "$ProjectRoot\app",

    "$ProjectRoot\doc"
)

foreach ($Dir in $Dirs) {
    if (-not (Test-Path $Dir)) {
        New-Item -ItemType Directory -Path $Dir | Out-Null
        Write-Host "[DIR ] $Dir" -ForegroundColor Green
    }
    else {
        Write-Host "[SKIP] $Dir already exists" -ForegroundColor DarkYellow
    }
}

# 空文件列表
$Files = @(
    "$ProjectRoot\CMakeLists.txt",

    "$ProjectRoot\cmake\toolchain-arm-none-eabi-gcc.cmake",
    "$ProjectRoot\cmake\mcu-common.cmake",
    "$ProjectRoot\cmake\mcu-stm32f103.cmake",
    "$ProjectRoot\cmake\mcu-gd32f103.cmake",

    "$ProjectRoot\ld\stm32f103\STM32F103C8TX_FLASH.ld",
    "$ProjectRoot\ld\gd32f103\GD32F103C8TX_FLASH.ld",

    "$ProjectRoot\startup\stm32f103\startup_stm32f103xb.s",
    "$ProjectRoot\startup\stm32f103\system_stm32f1xx.c",
    "$ProjectRoot\startup\gd32f103\startup_gd32f10x_md.s",
    "$ProjectRoot\startup\gd32f103\system_gd32f10x.c",

    "$ProjectRoot\startup\CMakeLists.txt",

    "$ProjectRoot\bsp\CMakeLists.txt",
    "$ProjectRoot\bsp\include\bsp_clock.h",
    "$ProjectRoot\bsp\include\bsp_gpio.h",
    "$ProjectRoot\bsp\include\bsp_uart.h",
    "$ProjectRoot\bsp\include\bsp_systick.h",
    "$ProjectRoot\bsp\include\bsp_iwdg.h",

    "$ProjectRoot\bsp\src\common\bsp_delay.c",
    "$ProjectRoot\bsp\src\stm32f103\bsp_clock_stm32f103.c",
    "$ProjectRoot\bsp\src\stm32f103\bsp_gpio_stm32f103.c",
    "$ProjectRoot\bsp\src\stm32f103\bsp_uart_stm32f103.c",
    "$ProjectRoot\bsp\src\gd32f103\bsp_clock_gd32f103.c",
    "$ProjectRoot\bsp\src\gd32f103\bsp_gpio_gd32f103.c",
    "$ProjectRoot\bsp\src\gd32f103\bsp_uart_gd32f103.c",

    "$ProjectRoot\middleware\CMakeLists.txt",
    "$ProjectRoot\middleware\uart\CMakeLists.txt",
    "$ProjectRoot\middleware\ch395\CMakeLists.txt",

    "$ProjectRoot\app\CMakeLists.txt",
    "$ProjectRoot\app\main.c",

    "$ProjectRoot\doc\README.md"
)

foreach ($File in $Files) {
    if (-not (Test-Path $File)) {
        New-Item -ItemType File -Path $File | Out-Null
        Write-Host "[FILE] $File" -ForegroundColor Green
    }
    else {
        Write-Host "[SKIP] $File already exists" -ForegroundColor DarkYellow
    }
}

Write-Host ""
Write-Host "SideSlip project tree created successfully." -ForegroundColor Cyan
