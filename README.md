<div align="center">

# 静默注册表导入程序
*A lightweight, silent Windows registry importer*

[![Version](https://img.shields.io/badge/version-1.0.1-blue.svg)](https://github.com/MisonL/reg_import_tool)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](#)
[![C++](https://img.shields.io/badge/C++-11-blue.svg)](https://isocpp.org/)
[![Size](https://img.shields.io/badge/size-950KB-brightgreen.svg)](#)

**作者**: Mison
**联系方式**: 1360962086@qq.com

> **⚠️ 重要说明**: 这是一个Windows原生程序，使用Windows API开发。可以在macOS上使用交叉编译器编译，但生成的可执行文件只能在Windows系统上运行。

</div>

## ✨ 功能特点

| 特性 | 描述 |
|------|------|
| 🔇 **完全静默** | 无弹窗、无托盘图标，后台运行 |
| 📁 **灵活输入** | 支持单文件、多文件、通配符批量导入 |
| 🔧 **调试支持** | 详细日志记录，便于问题排查 |
| 📦 **零依赖** | 静态链接，单文件可运行 |
| 💾 **超小体积** | 优化后仅950KB |
| ✅ **高兼容** | Windows 10/11 完美支持 |
| 🛡️ **安全可靠** | RAII资源管理，线程安全 |
| 📏 **代码规范** | 符合C++ Core Guidelines |

---

## 🚀 快速开始

### 基本使用
```batch
# 导入默认文件
reg_import_silent.exe
```

### 高级用法
```batch
# 导入指定文件
reg_import_silent.exe test1.reg

# 批量导入（通配符）
reg_import_silent.exe *.reg

# 调试模式（生成日志）
reg_import_silent.exe --debug test.reg
```

## 使用方法

### 基本用法
```
reg_import_silent.exe                           # 默认导入disable_local_network_access.reg
```

### 指定文件
```
reg_import_silent.exe test1.reg                  # 导入指定文件
reg_import_silent.exe "C:\path\to\file.reg"      # 导入指定路径的文件
```

### 通配符支持
```
reg_import_silent.exe *.reg                      # 导入当前目录所有reg文件
reg_import_silent.exe test*.reg                  # 导入以test开头的reg文件
reg_import_silent.exe C:\path\to\*.reg           # 导入指定目录下所有reg文件
```

### 多文件导入
```
reg_import_silent.exe test1.reg test2.reg        # 导入多个指定文件
reg_import_silent.exe *.reg backup\*.reg         # 导入多个模式的文件
```

### 调试模式
```
reg_import_silent.exe --debug                    # 调试模式导入默认文件
reg_import_silent.exe --debug test1.reg          # 调试模式导入指定文件
reg_import_silent.exe --debug *.reg              # 调试模式批量导入
```

调试模式会在程序目录生成 `reg_import_debug.log` 日志文件，记录详细的执行过程。

## 🛠️ 编译说明

### Windows平台

```batch
# 一键编译（自动检测编译器）
compile.bat
```

**支持编译器**:
- ✅ Visual Studio (MSVC) - 推荐
- ✅ MinGW-w64

---

### macOS平台（交叉编译）

#### 1️⃣ 安装交叉编译器

```bash
# 安装 mingw-w64
brew install mingw-w64
```

#### 2️⃣ 编译程序

**📌 推荐方式 - 自动化脚本**

```bash
chmod +x compile_macos.sh
./compile_macos.sh
```

**📌 手动编译**

```bash
# 最小体积编译（推荐）- 950KB
x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
  -fdata-sections -ffunction-sections -Wl,--gc-sections \
  -o reg_import_silent.exe reg_import_silent.cpp \
  -static-libgcc -static-libstdc++ -mwindows

# 完整编译（包含版本信息）
x86_64-w64-mingw32-windres -i version.rc -O coff -o version.o
x86_64-w64-mingw32-g++ -std=c++11 -Os -s -flto -fmerge-constants \
  -fdata-sections -ffunction-sections -Wl,--gc-sections \
  -o reg_import_silent.exe reg_import_silent.cpp version.o \
  -static-libgcc -static-libstdc++ -mwindows
rm version.o
```

**编译优化选项说明**:
- `-Os` - 优化文件大小
- `-s` - 剥离符号表和调试信息
- `-flto` - 链接时优化（LTO）
- 文件大小：12MB → **950KB** 📉

> **⚠️ 注意**: 编译生成的 `reg_import_silent.exe` 是Windows可执行文件，只能在Windows系统上运行。macOS上无法直接执行此程序。

## 🔧 技术实现

### 核心特性
- 💻 **原生开发** - C++ + Windows API
- 🔇 **静默运行** - 无控制台、无界面
- 📁 **通配符支持** - 灵活的批量文件处理
- 🛡️ **安全机制** - RAII资源管理
- 🔒 **线程安全** - 线程安全的日志功能
- 📏 **代码规范** - 严格遵循C++ Core Guidelines

---

## 💎 代码质量

<div align="center">

| 指标 | 状态 |
|------|------|
| ⚠️ 编译警告 | 零警告 ✅ |
| ❌ 编译错误 | 零错误 ✅ |
| 🔒 内存安全 | RAII模式 ✅ |
| 🔄 线程安全 | 完全安全 ✅ |
| 📏 代码规范 | C++ Core Guidelines ✅ |
| ⚡ 性能优化 | LTO优化 ✅ |
| 💾 文件大小 | 950KB (最小化) ✅ |

</div>

**编译标准**:
```bash
-std=c++11 -Wall -Wextra -Wpedantic
```

**优化特性**:
- ✅ Link Time Optimization (LTO)
- ✅ 符号表剥离
- ✅ 未使用代码消除
- ✅ 代码段优化
- ✅ 常量折叠

---

## 📄 许可证

本项目基于 [MIT License](LICENSE) 开源。

## 👤 作者

**Mison**
- 📧 邮箱：1360962086@qq.com

---

<div align="center">

**[⬆ 回到顶部](#静默注册表导入程序)**

*如果这个项目对您有帮助，请给个⭐ Star！*

</div>