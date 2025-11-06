@echo off
REM 静默注册表导入程序 - 编译器安装脚本
REM 作者: Mison
REM 联系方式: 1360962086@qq.com
REM 许可证: MIT License

echo 正在下载TDM-GCC MinGW编译器...
echo.

powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/jmeubank/tdm-gcc/releases/download/v10.3.0-tdm64-2/tdm64-gcc-10.3.0-2.exe' -OutFile 'tdm-gcc-installer.exe'}"

if exist tdm-gcc-installer.exe (
    echo 下载完成！正在静默安装...
    tdm-gcc-installer.exe /S
    echo 安装完成！
) else (
    echo 下载失败！请手动安装MinGW编译器。
    echo.
    echo 下载地址：
    echo https://jmeubank.github.io/tdm-gcc/
    echo.
    echo 或者访问：https://www.mingw-w64.org/downloads/
)

pause