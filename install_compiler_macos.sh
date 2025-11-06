#!/bin/bash

# ========================================================================
# 静默注册表导入程序 - macOS开发环境搭建脚本
# 作者: Mison
# 联系方式: 1360962086@qq.com
# 许可证: MIT License
# ========================================================================

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印带颜色的标题
print_header() {
    clear
    echo -e "${CYAN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                macOS开发环境搭建脚本                         ║"
    echo "║             Reg Import Tool 开发环境配置                      ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo
}

# 打印带颜色的信息
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# 打印成功信息
print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

# 打印警告信息
print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

# 打印错误信息
print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# 打印分隔线
print_separator() {
    echo -e "${PURPLE}══════════════════════════════════════════════════════════════${NC}"
}

# 检测系统信息
detect_system() {
    print_info "正在检测系统信息..."
    echo "  - macOS版本: $(sw_vers -productVersion)"
    echo "  - 系统架构: $(uname -m)"
    echo "  - 处理器: $(sysctl -n machdep.cpu.brand_string)"
    echo
}

# 检查Homebrew
check_homebrew() {
    print_info "检查Homebrew..."

    if command -v brew &> /dev/null; then
        BREW_VERSION=$(brew --version | head -n 1)
        print_success "Homebrew已安装: $BREW_VERSION"
        print_info "检查mingw-w64..."
        if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
            MINGW_VERSION=$(x86_64-w64-mingw32-g++ --version | head -n 1)
            print_success "mingw-w64已安装: $MINGW_VERSION"
            echo
            print_success "检测到mingw-w64，可以直接编译！"
            echo
            print_info "运行 './compile_macos.sh' 即可开始编译"
            echo
            exit 0
        else
            print_warning "mingw-w64未安装"
            echo
            return 1
        fi
    else
        print_error "Homebrew未安装"
        echo
        return 1
    fi
}

# 安装Homebrew
install_homebrew() {
    print_separator
    print_info "安装Homebrew"
    print_separator
    echo

    print_info "请按以下步骤操作："
    echo
    echo "  1. 在终端中运行以下命令："
    echo
    echo -e "${YELLOW}    /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    echo
    echo "  2. 按照提示完成安装"
    echo "  3. 将Homebrew添加到PATH（如有必要）"
    echo "  4. 重新运行此脚本"
    echo
    print_info "或者访问官方文档: https://brew.sh/"
    echo

    # 尝试自动安装
    read -p "是否尝试自动安装Homebrew? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "正在安装Homebrew..."
        echo
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

        if command -v brew &> /dev/null; then
            print_success "Homebrew安装成功！"
            echo
            return 0
        else
            print_error "Homebrew安装失败！"
            echo
            return 1
        fi
    else
        print_info "请手动安装Homebrew后重新运行此脚本"
        exit 1
    fi
}

# 安装mingw-w64
install_mingw() {
    print_separator
    print_info "安装mingw-w64"
    print_separator
    echo

    print_info "使用Homebrew安装mingw-w64..."
    echo
    echo "此过程可能需要几分钟时间，请耐心等待..."
    echo

    # 更新Homebrew
    print_info "更新Homebrew缓存..."
    brew update

    # 安装mingw-w64
    print_info "正在下载并安装mingw-w64..."
    brew install mingw-w64

    # 验证安装
    echo
    if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
        MINGW_VERSION=$(x86_64-w64-mingw32-g++ --version | head -n 1)
        print_success "mingw-w64安装成功！"
        echo
        print_info "版本信息: $MINGW_VERSION"
        echo
        print_success "现在可以编译Windows程序了！"
        echo

        # 检查PATH
        if echo $PATH | grep -q "/opt/homebrew/bin\|/usr/local/bin"; then
            print_success "PATH配置正确"
        else
            print_warning "请确保以下路径在PATH中："
            echo "  - /opt/homebrew/bin (Apple Silicon)"
            echo "  - /usr/local/bin (Intel)"
        fi

        echo
        print_info "使用方法："
        echo "  1. 运行 './compile_macos.sh' 编译"
        echo "  2. 编译完成后在Windows上测试可执行文件"
        echo

        return 0
    else
        print_error "mingw-w64安装失败！"
        echo
        print_info "请尝试手动安装："
        echo "  brew install mingw-w64"
        echo
        return 1
    fi
}

# 显示使用说明
show_usage() {
    print_separator
    print_info "使用说明"
    print_separator
    echo
    echo "编译步骤："
    echo
    echo "  1. 进入项目目录"
    echo "  2. 运行编译脚本："
    echo
    echo -e "     ${YELLOW}./compile_macos.sh${NC}"
    echo
    echo "  3. 编译完成后将生成："
    echo
    echo "     ${GREEN}reg_import_silent.exe${NC}  (Windows可执行文件)"
    echo
    echo "  4. 将可执行文件传输到Windows系统测试"
    echo
    echo "编译选项："
    echo "  - 最小体积优化 (默认)"
    echo "  - 零警告编译"
    echo "  - 静态链接"
    echo "  - 跨平台支持"
    echo
    echo "手动编译："
    echo "  x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto \\"
    echo "    -fdata-sections -ffunction-sections -Wl,--gc-sections \\"
    echo "    -o reg_import_silent.exe reg_import_silent.cpp \\"
    echo "    -static-libgcc -static-libstdc++ -mwindows"
    echo
}

# 主函数
main() {
    print_header
    detect_system
    print_separator

    # 检查现有环境
    if check_homebrew; then
        return 0
    fi

    echo
    print_separator
    print_info "未检测到mingw-w64"
    print_separator
    echo

    echo "请选择操作："
    echo
    echo "  1) 安装Homebrew (如果没有)"
    echo "  2) 安装mingw-w64 (需要Homebrew)"
    echo "  3) 显示使用说明"
    echo "  0) 退出"
    echo

    while true; do
        read -p "请输入选择 (0-3): " choice
        case $choice in
            0)
                echo
                print_info "退出安装"
                exit 0
                ;;
            1)
                echo
                if install_homebrew; then
                    echo
                    read -p "是否现在安装mingw-w64? (y/n): " -n 1 -r
                    echo
                    if [[ $REPLY =~ ^[Yy]$ ]]; then
                        install_mingw
                    fi
                fi
                break
                ;;
            2)
                echo
                install_mingw
                break
                ;;
            3)
                echo
                show_usage
                break
                ;;
            *)
                echo
                print_error "无效选择，请重试"
                ;;
        esac
    done

    echo
    print_separator
    print_success "开发环境配置完成！"
    print_separator
    echo
}

# 运行主函数
main "$@"
