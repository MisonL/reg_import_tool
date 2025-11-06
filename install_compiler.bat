@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM ========================================================================
REM 静默注册表导入程序 - Windows开发环境搭建脚本
REM 作者: Mison
REM 联系方式: 1360962086@qq.com
REM 许可证: MIT License
REM ========================================================================

title Windows开发环境搭建脚本
color 0A

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║                  Windows开发环境搭建脚本                      ║
echo ║              Reg Import Tool 开发环境配置                      ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ⚠️  提示: 建议以管理员权限运行此脚本以确保正常安装
    echo.
)

REM 检测系统信息
echo 🔍 正在检测系统信息...
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo   - Windows版本: %VERSION%

for /f "tokens=2 delims=:" %%i in ('systeminfo ^| findstr /c:"System Type"') do set ARCH=%%i
echo   - 系统架构:%ARCH%

REM 检测已安装的编译器
echo.
echo 🔍 检查已安装的编译器...

REM 检查Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✅ Visual Studio (MSVC) 已安装
    set VS_FOUND=1
) else (
    echo   ❌ Visual Studio (MSVC) 未安装
    set VS_FOUND=0
)

REM 检查MinGW
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✅ MinGW-w64 已安装
    set MINGW_FOUND=1
) else (
    echo   ❌ MinGW-w64 未安装
    set MINGW_FOUND=0
)

REM 检查TDM-GCC
if exist "C:\TDM-GCC-64\bin\g++.exe" (
    echo   ✅ TDM-GCC 已安装
    set TDM_FOUND=1
    set PATH=C:\TDM-GCC-64\bin;%PATH%
) else (
    echo   ❌ TDM-GCC 未安装
    set TDM_FOUND=0
)

REM 如果已找到编译器，询问是否继续
if %VS_FOUND% EQU 1 (
    echo.
    echo ✅ 检测到Visual Studio，可以直接编译！
    echo 💡 运行 compile.bat 即可开始编译
    pause
    exit /b 0
)

if %MINGW_FOUND% EQU 1 (
    echo.
    echo ✅ 检测到MinGW，可以直接编译！
    echo 💡 运行 compile.bat 即可开始编译
    pause
    exit /b 0
)

if %TDM_FOUND% EQU 1 (
    echo.
    echo ✅ 检测到TDM-GCC，可以直接编译！
    echo 💡 运行 compile.bat 即可开始编译
    pause
    exit /b 0
)

echo.
echo ═══════════════════════════════════════════════════════════════
echo                    未检测到编译器
echo ═══════════════════════════════════════════════════════════════
echo.
echo 请选择要安装的编译器：
echo.
echo   1️⃣  Visual Studio Community 2022 (推荐 - 包含MSVC编译器)
echo       - 微软官方C++编译器
echo       - 优化支持完善
echo       - IDE和工具链完整
echo       - 体积较大 (~3GB)
echo.
echo   2️⃣  MinGW-w64 (轻量级 - 纯命令行)
echo       - GCC编译器 Windows移植版
echo       - 轻量级，易安装
echo       - 纯命令行操作
echo       - 体积小 (~100MB)
echo.
echo   3️⃣  TDM-GCC (稳定版 - 推荐)
echo       - 基于GCC的Windows编译器
echo       - 预配置了Windows特定设置
echo       - 稳定可靠
echo       - 体积中等 (~150MB)
echo.
echo   0️⃣  退出
echo.

:CHOICE
set /p CHOICE="请输入选择 (0-3): "

if "%CHOICE%"=="0" (
    echo.
    echo 👋 退出安装
    pause
    exit /b 0
) else if "%CHOICE%"=="1" goto INSTALL_VS
if "%CHOICE%"=="2" goto INSTALL_MINGW
if "%CHOICE%"=="3" goto INSTALL_TDM
if "%CHOICE%"=="" (
    echo ❌ 无效选择，请重试
    goto CHOICE
) else (
    echo ❌ 无效选择，请重试
    goto CHOICE
)

:INSTALL_VS
echo.
echo ═══════════════════════════════════════════════════════════════
echo                    安装 Visual Studio
echo ═════════════════════════워크로드         ═════════════════════════════
echo.
echo 📥 正在打开Visual Studio下载页面...
echo.
echo 请按以下步骤操作：
echo   1. 下载 Visual Studio Community 2022 (免费)
echo   2. 运行安装程序
echo   3. 选择 "使用C++的桌面开发" 工作负载
echo   4. 完成安装
echo.
echo 🔗 下载地址:
echo    https://visualstudio.microsoft.com/zh-hans/vs/community/
echo.
echo 💡 安装完成后，请重新运行此脚本
pause
start "" "https://visualstudio.microsoft.com/zh-hans/vs/community/"
pause
exit /b 0

:INSTALL_MINGW
echo.
echo ═══════════════════════════════════════════════════════════════
echo                    安装 MinGW-w64
echo ═══════════════════════════════════════════════════════════════
echo.
echo 📥 正在下载 MinGW-w64 ...

REM 检测架构
if "%ARCH%"=="x64" (
    set MINGW_URL=https://github.com/niXman/mingw-builds-binaries/releases/download/x86_64-13.2.0-rev3-20231208/mingw64-13.2.0-rev3-20231208.7z
    set MINGW_DIR=C:\mingw64
    set MINGW_NAME=mingw64
) else (
    set MINGW_URL=https://github.com/niXman/mingw-builds-binaries/releases/download/x86_64-13.2.0-rev3-20231208/mingw32-13.2.0-rev3-20231208.7z
    set MINGW_DIR=C:\mingw32
    set MINGW_NAME=mingw32
)

powershell -Command "try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%MINGW_URL%' -OutFile 'mingw.7z' -UseBasicParsing } catch { exit 1 }"

if not exist "mingw.7z" (
    echo ❌ 下载失败！
    echo.
    echo 🔗 请手动下载并安装：
    echo    https://www.mingw-w64.org/downloads/
    echo.
    pause
    exit /b 1
)

echo ✅ 下载完成
echo.
echo 📦 正在解压...
if not exist "C:\7zip" mkdir "C:\7zip"
echo   需要7zip工具来解压文件

REM 检查7zip
where 7z >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ 未找到7zip，正在下载...
    powershell -Command "try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://www.7-zip.org/a/7z2201-x64.exe' -OutFile '7z.exe' -UseBasicParsing } catch { exit 1 }"
    7z.exe /S /D=C:\7zip >nul 2>&1
)

REM 解压
7z x "mingw.7z" -o"%MINGW_DIR%" -y >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo ✅ 安装完成！
    set PATH=%MINGW_DIR%\bin;%PATH%
    echo.
    echo ⚠️  重要: 需要将以下路径添加到系统PATH环境变量:
    echo    %MINGW_DIR%\bin
    echo.
    echo 💡 请手动添加此路径，或重新启动命令提示符
) else (
    echo ❌ 解压失败！
    pause
    exit /b 1
)

del mingw.7z >nul 2>&1
echo.
echo ✅ MinGW-w64 安装完成！
echo 💡 现在可以运行 compile.bat 进行编译
pause
exit /b 0

:INSTALL_TDM
echo.
echo ═══════════════════════════════════════════════════════════════
echo                    安装 TDM-GCC
echo ═══════════════════════════════════════════════════════════════
echo.
echo 📥 正在下载 TDM-GCC ...

set TDM_URL=https://github.com/jmeubank/tdm-gcc/releases/download/v10.3.0-tdm64-2/tdm64-gcc-10.3.0-2.exe

powershell -Command "try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%TDM_URL%' -OutFile 'tdm-gcc-installer.exe' -UseBasicParsing } catch { exit 1 }"

if not exist "tdm-gcc-installer.exe" (
    echo ❌ 下载失败！
    echo.
    echo 🔗 请手动下载并安装：
    echo    https://jmeubank.github.io/tdm-gcc/
    echo.
    pause
    exit /b 1
)

echo ✅ 下载完成
echo.
echo 📦 正在安装 TDM-GCC (静默模式)...
echo   - 这可能需要几分钟时间
echo   - 请耐心等待...

tdm-gcc-installer.exe /S >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo ✅ 安装完成！
    set PATH=C:\TDM-GCC-64\bin;%PATH%
    echo.
    echo ⚠️  如果提示找不到编译器，请重启命令提示符
) else (
    echo ❌ 安装失败！
    echo.
    echo 💡 请手动安装:
    echo    1. 找到 tdm-gcc-installer.exe
    echo    2. 双击运行
    echo    3. 按默认设置安装
)

del tdm-gcc-installer.exe >nul 2>&1
echo.
echo ✅ TDM-GCC 安装完成！
echo 💡 现在可以运行 compile.bat 进行编译
pause
exit /b 0