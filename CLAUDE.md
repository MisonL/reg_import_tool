# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个用C++编写的Windows静默注册表导入工具，用于无界面地批量导入注册表文件。项目支持MSVC和MinGW编译器，完全基于Windows API实现，无外部依赖。

**作者**: Mison
**版本**: 1.0.1
**许可证**: MIT License

> **重要说明**: 这是一个Windows原生程序，使用Windows API开发。虽然可以在macOS上使用交叉编译器编译，但生成的可执行文件只能在Windows系统上运行。

## 开发环境

### Windows平台

#### 编译器要求
- **Visual Studio** (推荐，MSVC编译器)
- **TDM-GCC/MinGW-w64** (备选方案)

### macOS平台（交叉编译）

在macOS上可以使用交叉编译器为Windows平台编译程序：

#### 安装交叉编译器

```bash
# 使用Homebrew安装mingw-w64
brew install mingw-w64
```

#### 交叉编译命令

```bash
# 编译Windows可执行文件
x86_64-w64-mingw32-g++ -o reg_import_silent.exe reg_import_silent.cpp \
  -static-libgcc -static-libstdc++ -lwinpthread \
  -mwindows

# 或者使用clang（如果安装了Windows SDK）
clang++ --target=x86_64-w64-mingw reg_import_silent.cpp -o reg_import_silent.exe \
  -static-libgcc -static-libstdc++ -mwindows
```

### 编译命令

#### Windows平台

```batch
# 安装开发环境（首次使用）
install_compiler.bat

# 自动编译（推荐）
compile.bat
```

**install_compiler.bat 功能**:
- 自动检测已安装的编译器
- 提供3种编译器选择：Visual Studio、MinGW-w64、TDM-GCC
- 自动下载并静默安装
- 配置环境变量

`compile.bat` 会自动：
1. 检测已安装的编译器（优先MSVC，其次MinGW）
2. 编译 `reg_import_silent.cpp`
3. 链接版本资源文件 `version.rc`
4. 生成可执行文件 `reg_import_silent.exe`

#### macOS平台

```bash
# 方式一：使用自动化脚本（推荐）- 生成最小体积正式版
chmod +x compile_macos.sh
./compile_macos.sh

# 方式二：手动交叉编译（正式版 - 最小体积）
x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
  -fdata-sections -ffunction-sections -Wl,--gc-sections \
  -o reg_import_silent.exe reg_import_silent.cpp \
  -static-libgcc -static-libstdc++ -mwindows

# 完整编译（包含版本信息和最小体积优化）
x86_64-w64-mingw32-windres -i version.rc -O coff -o version.o
x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
  -fdata-sections -ffunction-sections -Wl,--gc-sections \
  -o reg_import_silent.exe reg_import_silent.cpp version.o \
  -static-libgcc -static-libstdc++ -mwindows
rm version.o
```

**install_compiler_macos.sh 功能**:
- 自动检测系统和Homebrew
- 安装或验证mingw-w64交叉编译器
- 配置环境变量
- 提供详细的使用说明

### 运行方式

```batch
# 基本使用 - 导入默认文件
reg_import_silent.exe

# 指定文件
reg_import_silent.exe test.reg

# 通配符批量导入
reg_import_silent.exe *.reg

# 调试模式（生成日志）
reg_import_silent.exe --debug test.reg
```

## 代码架构

### 核心文件
- **`reg_import_silent.cpp`** (330行) - 主程序，包含所有业务逻辑
- **`version.rc`** - Windows资源文件，存储版本信息
- **`compile.bat`** - Windows平台编译脚本，支持MSVC和MinGW（已优化）
- **`compile_macos.sh`** - macOS平台交叉编译脚本（已优化）
- **`install_compiler.bat`** - Windows平台开发环境搭建脚本
- **`install_compiler_macos.sh`** - macOS平台开发环境搭建脚本
- **`disable_local_network_access.reg`** - 默认注册的注册表文件（已忽略）

### 关键函数

**程序入口** (`WinMain`)
- 解析命令行参数
- 处理 `--debug`、`--help` 参数
- 支持多文件和通配符匹配
- 调度文件导入流程

**核心导入逻辑** (`ImportRegFile`)
- 使用 `reg import` 命令静默导入注册表
- 通过 `CreateProcess` 创建隐藏进程
- 等待进程完成并检查返回码
- 失败时记录错误信息

**调试支持** (`WriteLog`)
- 仅在 `--debug` 模式下启用
- 生成带时间戳的日志文件 `reg_import_debug_YYYYMMDD_HHMMSS.log`
- 自动清理旧日志（保留最近5个）

**文件处理** (`FindFiles`)
- 解析命令行中的通配符（`*` 和 `?`）
- 使用 `FindFirstFile/FindNextFile` 搜索匹配文件
- 支持相对路径和绝对路径

### 关键特性

1. **静默运行**: 默认无控制台窗口，完全后台执行
2. **调试模式**: `--debug` 参数启用详细日志和控制台输出
3. **批量处理**: 支持多文件和通配符批量导入
4. **错误处理**: 捕获进程创建和执行错误，返回适当退出码
5. **Windows集成**: 使用原生Windows API，无需额外依赖

## 开发指南

### 修改版本号
需要同时更新以下位置：
1. `reg_import_silent.cpp:25-28` - 版本宏定义
2. `version.rc:2-3` - 文件和产品版本
3. `README.md:6` - 项目版本

### 添加新功能
程序使用简单的过程式架构，主要逻辑集中在 `WinMain` 函数中。如需添加新功能：

1. 在文件顶部添加全局变量（如 `g_debugMode`）
2. 实现对应的处理函数
3. 在 `WinMain` 中添加参数解析逻辑

### 编译注意事项

#### Windows平台
- **MSVC**: 使用 `/EHsc /MT /O2` 编译选项，链接为Windows子系统
- **MinGW**: 使用 `-mwindows -static-libgcc -static-libstdc++` 选项
- 版本资源文件通过 `windres` 编译为 `.o` 文件后链接

#### macOS平台
- **交叉编译器**: 使用 `mingw-w64` 提供的交叉编译器工具链
- **工具链前缀**: `x86_64-w64-mingw32-` (64位Windows)
- **Windres**: 交叉版本的资源编译器为 `x86_64-w64-mingw32-windres`
- **静态链接**: 建议使用 `-static-libgcc -static-libstdc++` 避免依赖问题
- **子系统**: 使用 `-mwindows` 创建Windows窗口程序（无控制台）
- **标准**: 使用 `-std=c++11` 启用现代C++特性

#### 代码质量
本项目已通过严格编译检查：
- `-Wall -Wextra -Wpedantic`: 启用所有警告
- 零警告编译通过
- 符合C++ Core Guidelines最佳实践
- RAII模式确保资源安全
- 线程安全的日志记录

### 测试建议
使用 `--debug` 模式进行测试，查看生成的日志文件验证：
- 文件路径解析是否正确
- 通配符匹配是否完整
- 注册表导入是否成功
- 错误处理是否到位

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。
