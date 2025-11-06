#!/bin/bash
# macOS平台交叉编译脚本
# 作者: Mison
# 联系方式: 1360962086@qq.com
# 许可证: MIT License

echo "=== 静默注册表导入程序 - macOS交叉编译（正式版优化）==="
echo ""

# 检查mingw-w64是否安装
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "错误: 未找到mingw-w64交叉编译器"
    echo ""
    echo "请先安装:"
    echo "  brew install mingw-w64"
    echo ""
    exit 1
fi

echo "✓ 找到mingw-w64交叉编译器"

# 检查源代码文件
if [ ! -f "reg_import_silent.cpp" ]; then
    echo "错误: 未找到reg_import_silent.cpp"
    exit 1
fi

echo "✓ 找到源代码文件"

# 编译版本资源文件
if [ -f "version.rc" ]; then
    echo "✓ 找到版本资源文件，正在编译..."
    x86_64-w64-mingw32-windres -i version.rc -O coff -o version.o
    if [ $? -ne 0 ]; then
        echo "错误: 版本资源文件编译失败"
        exit 1
    fi
    USE_VERSION_RESOURCE=1
else
    echo "⚠ 未找到version.rc，将不包含版本信息"
    USE_VERSION_RESOURCE=0
fi

# 编译主程序（正式版 - 最小体积优化）
echo "正在编译正式版本..."
if [ $USE_VERSION_RESOURCE -eq 1 ]; then
    x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
        -fdata-sections -ffunction-sections -Wl,--gc-sections \
        -Wall -Wextra -Wpedantic -o reg_import_silent.exe reg_import_silent.cpp version.o \
        -static-libgcc -static-libstdc++ -mwindows
    COMPILE_RESULT=$?
    rm -f version.o
else
    x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
        -fdata-sections -ffunction-sections -Wl,--gc-sections \
        -Wall -Wextra -Wpedantic -o reg_import_silent.exe reg_import_silent.cpp \
        -static-libgcc -static-libstdc++ -mwindows
    COMPILE_RESULT=$?
fi

# 检查编译结果
if [ $COMPILE_RESULT -eq 0 ]; then
    echo ""
    echo "✓ 编译成功！"
    echo ""
    echo "生成文件: reg_import_silent.exe"
    if [ -f "reg_import_silent.exe" ]; then
        FILE_SIZE=$(ls -lh reg_import_silent.exe | awk '{print $5}')
        FILE_TYPE=$(file reg_import_silent.exe | cut -d: -f2)
        echo "文件大小: $FILE_SIZE"
        echo "文件类型: $FILE_TYPE"
        echo ""
        echo "优化特性:"
        echo "  - LTO (链接时优化)"
        echo "  - 代码和数据段分离"
        echo "  - 未使用代码消除"
        echo "  - 符号表剥离"
        echo ""
        echo "注意: 此可执行文件只能在Windows系统上运行"
    fi
else
    echo ""
    echo "✗ 编译失败！"
    exit 1
fi
