@echo off
REM 静默注册表导入程序 - 编译脚本（正式版优化）
REM 作者: Mison
REM 联系方式: 1360962086@qq.com
REM 许可证: MIT License

echo 正在编译静默注册表导入程序（正式版优化）...
echo.

REM 设置TDM-GCC路径（如果安装到默认位置）
set PATH=C:\TDM-GCC-64\bin;%PATH%

REM 检查是否安装了Visual Studio
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo 使用MSVC编译器编译（优化版）...
    cl /O2 /DNDEBUG /EHsc /MT /GL reg_import_silent.cpp /Fe:reg_import_silent.exe /link /SUBSYSTEM:WINDOWS /LTCG
    if %ERRORLEVEL% EQU 0 (
        echo 编译成功！生成文件：reg_import_silent.exe
        echo 优化特性：
        echo   - 运行时库多线程 (/MT)
        echo   - 全程序优化 (/GL /LTCG)
        echo   - 符号表已优化
    ) else (
        echo MSVC编译失败！
    )
    goto :end
)

REM 检查是否安装了MinGW
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo 使用MinGW编译器编译（最小体积优化）...
    if exist version.rc (
        windres -i version.rc -O coff -o version.o
        g++ -std=c++11 -Os -s -flto -fmerge-constants -fdata-sections -ffunction-sections -Wl,--gc-sections -o reg_import_silent.exe reg_import_silent.cpp version.o -static-libgcc -static-libstdc++ -mwindows
        del version.o 2>nul
    ) else (
        g++ -std=c++11 -Os -s -flto -fmerge-constants -fdata-sections -ffunction-sections -Wl,--gc-sections -o reg_import_silent.exe reg_import_silent.cpp -static-libgcc -static-libstdc++ -mwindows
    )
    if %ERRORLEVEL% EQU 0 (
        echo 编译成功！生成文件：reg_import_silent.exe
        echo 优化特性：
        echo   - LTO (链接时优化)
        echo   - 代码和数据段分离
        echo   - 未使用代码消除
        echo   - 符号表剥离
        echo   - 文件大小最小化
    ) else (
        echo MinGW编译失败！
    )
    goto :end
)

echo 未找到编译器！请安装Visual Studio或MinGW。
echo.
echo 安装建议：
echo 1. Visual Studio Community: https://visualstudio.microsoft.com/downloads/
echo 2. TDM-GCC: https://jmeubank.github.io/tdm-gcc/
echo.

:end
pause