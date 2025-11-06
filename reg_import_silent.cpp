/*
 * 静默注册表导入程序
 * 作者: Mison
 * 联系方式: 1360962086@qq.com
 * 许可证: MIT License
 * 版本: 1.1.0
 *
 * 功能特点:
 * - 完全静默运行，无任何弹窗
 * - 支持命令行参数指定reg文件路径
 * - 支持通配符（*和?）批量导入
 * - 支持调试模式，详细日志记录
 * - 新增：注册表查询功能（--query-registry）
 * - 无外部依赖项，单文件运行
 * - 兼容Windows 10/11
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <memory>
#include <cstring>

// 版本信息
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_BUILD 0
#define VERSION_STRING "1.1.0"

// 全局调试模式标志
bool g_debugMode = false;
std::ofstream g_logFile;

// 注册表查询模式标志
bool g_queryMode = false;
std::string g_queryPath = "";

// RAII类用于安全处理Windows句柄
struct HandleRAII {
    HANDLE h;
    explicit HandleRAII(HANDLE handle) : h(handle) {}
    ~HandleRAII() {
        if (h != INVALID_HANDLE_VALUE && h != NULL) {
            CloseHandle(h);
        }
    }
    // 禁止拷贝
    HandleRAII(const HandleRAII&) = delete;
    HandleRAII& operator=(const HandleRAII&) = delete;
};

// 日志记录函数 - 线程安全版本
void WriteLog(const std::string& message) {
    if (g_debugMode && g_logFile.is_open()) {
        time_t now = std::time(nullptr);
        tm localTime;
        // 使用线程安全的本地时间函数
        if (localtime_s(&localTime, &now) == 0) {
            char timestamp[32];
            if (std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &localTime) > 0) {
                g_logFile << "[" << timestamp << "] " << message << std::endl;
                g_logFile.flush();

                // 在调试模式下也输出到控制台（如果存在）
                HWND consoleWindow = GetConsoleWindow();
                if (consoleWindow != NULL) {
                    std::cout << "[" << timestamp << "] " << message << std::endl;
                }
            }
        }
    }
}

// 显示帮助信息
void ShowHelp() {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(65001);
    
    const char* helpText =
        "Silent Registry Import Tool v" VERSION_STRING "\n"
        "Author: Mison (1360962086@qq.com)\n"
        "License: MIT License\n\n"
        "Usage: reg_import_silent.exe [options] [file_paths...]\n\n"
        "Options:\n"
        "  --debug          Enable debug mode, generate detailed logs\n"
        "  --query-registry <path>  Query registry path (auto-enables debug mode)\n"
        "  --help           Show this help information\n\n"
        "File Paths:\n"
        "  Support single or multiple reg file paths\n"
        "  Support wildcards (* and ?) for batch matching\n\n"
        "Examples:\n"
        "  reg_import_silent.exe                           # Import default file\n"
        "  reg_import_silent.exe test1.reg                  # Import specified file\n"
        "  reg_import_silent.exe *.reg                      # Import all reg files\n"
        "  reg_import_silent.exe --debug test1.reg          # Debug mode import\n"
        "  reg_import_silent.exe --query-registry HKLM\\SOFTWARE\\Microsoft  # Query registry\n"
        "  reg_import_silent.exe --help                     # Show help\n\n"
        "Registry Path Examples:\n"
        "  HKLM\\SOFTWARE\\Microsoft          (HKEY_LOCAL_MACHINE)\n"
        "  HKCU\\Software                     (HKEY_CURRENT_USER)\n"
        "  HKCR\\CLSID                        (HKEY_CLASSES_ROOT)\n"
        "  HKU\\.DEFAULT                      (HKEY_USERS)\n"
        "  HKCC\\SYSTEM                       (HKEY_CURRENT_CONFIG)\n\n"
        "Notes:\n"
        "  - Program runs silently by default, no interface\n"
        "  - Debug mode generates timestamped log files\n"
        "  - Query mode shows all subkeys and values recursively\n"
        "  - Support Windows 10/11\n"
        "  - No external dependencies\n"
        "  - Open source under MIT License\n";
    
    // 创建控制台窗口显示帮助
    AllocConsole();
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    std::cout << helpText << std::endl;
    std::cout << "\nPress any key to exit..." << std::endl;
    system("pause > nul");
    FreeConsole();
}

// 清理旧日志文件（保留最近5个）
void CleanOldLogs() {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        return;
    }

    char* lastSlash = std::strrchr(exePath, '\\');
    if (lastSlash != NULL) {
        *lastSlash = '\0';
    }

    // 查找所有日志文件
    std::string searchPattern = std::string(exePath) + "\\reg_import_debug*.log";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::vector<std::string> logFiles;
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string filePath = std::string(exePath) + "\\" + findData.cFileName;
            logFiles.push_back(filePath);
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);

    // 如果日志文件超过5个，删除最旧的
    if (logFiles.size() <= 5) {
        return;
    }

    // 使用RAII确保文件句柄被正确关闭
    struct FileInfo {
        std::string path;
        FILETIME writeTime;
    };

    std::vector<FileInfo> files;
    files.reserve(logFiles.size());

    // 获取所有文件的修改时间
    for (const auto& filePath : logFiles) {
        HandleRAII hFile(CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
        if (hFile.h != INVALID_HANDLE_VALUE) {
            FILETIME ft;
            if (GetFileTime(hFile.h, NULL, NULL, &ft)) {
                files.push_back({filePath, ft});
            }
        }
    }

    // 按修改时间排序（最旧的在前）
    std::sort(files.begin(), files.end(),
              [](const FileInfo& a, const FileInfo& b) {
                  return CompareFileTime(&a.writeTime, &b.writeTime) < 0;
              });

    // 删除最旧的文件（保留最新的5个）
    size_t filesToDelete = files.size() - 5;
    for (size_t i = 0; i < filesToDelete; ++i) {
        DeleteFileA(files[i].path.c_str());
    }
}

// 递归查找匹配通配符的文件
void FindFiles(const std::string& pattern, std::vector<std::string>& files) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filePath = pattern;
                size_t lastSlash = filePath.find_last_of("\\/");
                if (lastSlash != std::string::npos) {
                    filePath = filePath.substr(0, lastSlash + 1) + findData.cFileName;
                } else {
                    filePath = findData.cFileName;
                }
                files.push_back(filePath);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
}

// 获取注册表数据类型名称
std::string GetRegTypeName(DWORD type) {
    if (type == REG_NONE) return "REG_NONE";
    if (type == REG_SZ) return "REG_SZ";
    if (type == REG_EXPAND_SZ) return "REG_EXPAND_SZ";
    if (type == REG_BINARY) return "REG_BINARY";
    if (type == REG_DWORD || type == REG_DWORD_LITTLE_ENDIAN) return "REG_DWORD";
    if (type == REG_DWORD_BIG_ENDIAN) return "REG_DWORD_BIG_ENDIAN";
    if (type == REG_LINK) return "REG_LINK";
    if (type == REG_MULTI_SZ) return "REG_MULTI_SZ";
    if (type == REG_RESOURCE_LIST) return "REG_RESOURCE_LIST";
    if (type == REG_FULL_RESOURCE_DESCRIPTOR) return "REG_FULL_RESOURCE_DESCRIPTOR";
    if (type == REG_RESOURCE_REQUIREMENTS_LIST) return "REG_RESOURCE_REQUIREMENTS_LIST";
    return "UNKNOWN";
}

// 格式化注册表值数据
std::string FormatRegValueData(DWORD type, const BYTE* data, DWORD dataSize) {
    if (data == NULL || dataSize == 0) {
        return "(empty)";
    }

    switch (type) {
        case REG_SZ:
        case REG_EXPAND_SZ: {
            return std::string((char*)data, dataSize - 1);
        }
        case REG_DWORD: {
            if (dataSize >= sizeof(DWORD)) {
                DWORD value = *((DWORD*)data);
                return "0x" + std::to_string(value) + " (" + std::to_string(value) + ")";
            }
            return "(invalid DWORD)";
        }
        case REG_BINARY: {
            std::string result;
            for (DWORD i = 0; i < dataSize && i < 256; i++) {
                char hex[4];
                snprintf(hex, sizeof(hex), "%02X ", data[i]);
                result += hex;
            }
            if (dataSize > 256) {
                result += "... (" + std::to_string(dataSize) + " bytes total)";
            }
            return result;
        }
        case REG_MULTI_SZ: {
            std::string result;
            std::string multiStr((char*)data, dataSize);
            for (size_t pos = 0; pos < multiStr.length(); ) {
                size_t nextPos = multiStr.find('\0', pos);
                if (nextPos == std::string::npos) {
                    result += multiStr.substr(pos);
                    break;
                }
                if (nextPos > pos) {
                    if (!result.empty()) result += "; ";
                    result += multiStr.substr(pos, nextPos - pos);
                }
                pos = nextPos + 1;
            }
            return result;
        }
        default: {
            std::string result = "(" + GetRegTypeName(type) + ", " + std::to_string(dataSize) + " bytes)";
            return result;
        }
    }
}

// 查询注册表路径下的所有信息
void QueryRegistry(const std::string& path, int indent = 0, bool isRoot = true) {
    std::string indentStr(indent * 2, ' ');

    // 解析根键简称
    HKEY rootKey = NULL;
    std::string subPath;
    std::string pathToUse = path;

    // 支持简化路径格式
    if (path.rfind("HKLM\\", 0) == 0 || path.rfind("HKEY_LOCAL_MACHINE\\", 0) == 0) {
        rootKey = HKEY_LOCAL_MACHINE;
        subPath = path.substr(path.find('\\') + 1);
    } else if (path.rfind("HKCU\\", 0) == 0 || path.rfind("HKEY_CURRENT_USER\\", 0) == 0) {
        rootKey = HKEY_CURRENT_USER;
        subPath = path.substr(path.find('\\') + 1);
    } else if (path.rfind("HKCR\\", 0) == 0 || path.rfind("HKEY_CLASSES_ROOT\\", 0) == 0) {
        rootKey = HKEY_CLASSES_ROOT;
        subPath = path.substr(path.find('\\') + 1);
    } else if (path.rfind("HKU\\", 0) == 0 || path.rfind("HKEY_USERS\\", 0) == 0) {
        rootKey = HKEY_USERS;
        subPath = path.substr(path.find('\\') + 1);
    } else if (path.rfind("HKCC\\", 0) == 0 || path.rfind("HKEY_CURRENT_CONFIG\\", 0) == 0) {
        rootKey = HKEY_CURRENT_CONFIG;
        subPath = path.substr(path.find('\\') + 1);
    } else {
        WriteLog("Error: Invalid registry path format: " + path);
        return;
    }

    // 打开注册表键
    HKEY hKey;
    LONG result = RegOpenKeyExA(rootKey, subPath.empty() ? NULL : subPath.c_str(), 0, KEY_READ, &hKey);

    if (result != ERROR_SUCCESS) {
        WriteLog("Error: Failed to open registry key: " + path + " (Error code: " + std::to_string(result) + ")");
        return;
    }

    WriteLog("Querying registry path: " + path);

    // 打印键路径
    std::cout << indentStr << (isRoot ? "[ " : "") << path << (isRoot ? " ]" : "") << std::endl;

    // 枚举键值
    DWORD valueIndex = 0;
    char valueName[MAX_PATH];
    DWORD valueNameSize;
    DWORD valueType;
    BYTE valueData[4096];
    DWORD valueDataSize;

    while (true) {
        valueNameSize = MAX_PATH;
        valueDataSize = sizeof(valueData);
        result = RegEnumValueA(hKey, valueIndex, valueName, &valueNameSize, NULL, &valueType, valueData, &valueDataSize);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (result == ERROR_SUCCESS) {
            std::string formattedValue = FormatRegValueData(valueType, valueData, valueDataSize);
            WriteLog("  Value: " + std::string(valueName) + " (" + GetRegTypeName(valueType) + ") = " + formattedValue);
            std::cout << indentStr << "  \"" << valueName << "\" = " << formattedValue << " (" << GetRegTypeName(valueType) << ")" << std::endl;
        }

        valueIndex++;
    }

    // 枚举子键
    DWORD subKeyIndex = 0;
    char subKeyName[MAX_PATH];
    DWORD subKeyNameSize;

    while (true) {
        subKeyNameSize = MAX_PATH;
        result = RegEnumKeyExA(hKey, subKeyIndex, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (result == ERROR_SUCCESS) {
            std::string subKeyFullPath = subPath.empty() ? subKeyName : subPath + "\\" + subKeyName;
            std::string subKeyFullPathWithRoot;
            if (path.rfind("HKLM\\", 0) == 0) subKeyFullPathWithRoot = "HKLM\\" + subKeyFullPath;
            else if (path.rfind("HKCU\\", 0) == 0) subKeyFullPathWithRoot = "HKCU\\" + subKeyFullPath;
            else if (path.rfind("HKCR\\", 0) == 0) subKeyFullPathWithRoot = "HKCR\\" + subKeyFullPath;
            else if (path.rfind("HKU\\", 0) == 0) subKeyFullPathWithRoot = "HKU\\" + subKeyFullPath;
            else if (path.rfind("HKCC\\", 0) == 0) subKeyFullPathWithRoot = "HKCC\\" + subKeyFullPath;
            else subKeyFullPathWithRoot = path + "\\" + subKeyName;

            // 递归查询子键
            QueryRegistry(subKeyFullPathWithRoot, indent + 1, false);
        }

        subKeyIndex++;
    }

    RegCloseKey(hKey);
}

// 静默导入单个reg文件
bool ImportRegFile(const std::string& regFilePath) {
    WriteLog("Starting registry import: " + regFilePath);
    std::string command = "reg import \"" + regFilePath + "\"";
    WriteLog("Executing command: " + command);
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));
    
    bool success = false;
    if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WriteLog("Process created successfully, waiting for completion...");
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode == 0) {
            success = true;
            WriteLog("Registry import successful");
        } else {
            WriteLog("Registry import failed, exit code: " + std::to_string(exitCode));
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        WriteLog("Process creation failed, error code: " + std::to_string(GetLastError()));
    }
    return success;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 标记未使用的参数（Windows API标准参数）
    (void)hInstance;
    (void)hPrevInstance;
    (void)nCmdShow;

    std::vector<std::string> regFiles;
    std::string cmdLine = lpCmdLine ? lpCmdLine : "";

    // 检查是否包含--help参数
    if (cmdLine.find("--help") != std::string::npos) {
        ShowHelp();
        return 0;
    }

    // 检查是否包含--query-registry参数（需要单独处理，因为后面有路径）
    size_t queryPos = cmdLine.find("--query-registry");
    if (queryPos != std::string::npos) {
        g_queryMode = true;
        g_debugMode = true;  // 自动启用debug模式

        // 提取查询路径
        size_t pathStart = queryPos + 15; // "--query-registry" 的长度
        if (pathStart < cmdLine.length()) {
            // 跳过路径前的空格
            while (pathStart < cmdLine.length() && cmdLine[pathStart] == ' ') {
                pathStart++;
            }
            // 提取路径直到下一个参数或行尾
            size_t pathEnd = cmdLine.find("--", pathStart);
            if (pathEnd == std::string::npos) {
                g_queryPath = cmdLine.substr(pathStart);
            } else {
                g_queryPath = cmdLine.substr(pathStart, pathEnd - pathStart);
            }
            // 去除路径尾部的空格
            while (!g_queryPath.empty() && g_queryPath.back() == ' ') {
                g_queryPath.pop_back();
            }
        }

        // 移除--query-registry及其路径参数
        cmdLine.erase(queryPos, 15 + g_queryPath.length());
        // 去除多余空格
        while (cmdLine.find("  ") != std::string::npos) {
            cmdLine.replace(cmdLine.find("  "), 2, " ");
        }
        if (!cmdLine.empty() && cmdLine[0] == ' ') {
            cmdLine.erase(0, 1);
        }
        if (!cmdLine.empty() && cmdLine[cmdLine.length() - 1] == ' ') {
            cmdLine.erase(cmdLine.length() - 1, 1);
        }

        WriteLog("=== Registry query mode enabled ===");
        WriteLog("Query path: " + g_queryPath);
    }

    // 检查是否包含--debug参数（支持任意位置）
    size_t debugPos = cmdLine.find("--debug");
    if (debugPos != std::string::npos) {
        g_debugMode = true;
        // 移除--debug参数
        cmdLine.erase(debugPos, 7);
        // 去除多余空格
        while (cmdLine.find("  ") != std::string::npos) {
            cmdLine.replace(cmdLine.find("  "), 2, " ");
        }
        if (!cmdLine.empty() && cmdLine[0] == ' ') {
            cmdLine.erase(0, 1);
        }
        if (!cmdLine.empty() && cmdLine[cmdLine.length() - 1] == ' ') {
            cmdLine.erase(cmdLine.length() - 1, 1);
        }

        // 如果不在查询模式，初始化日志
        if (!g_queryMode) {
            char exePath[MAX_PATH];
            GetModuleFileNameA(NULL, exePath, MAX_PATH);
            char* lastSlash = strrchr(exePath, '\\');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
            }

            CleanOldLogs();

            time_t now = time(0);
            tm localTime;
            if (localtime_s(&localTime, &now) == 0) {
                char timestamp[32];
                if (std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &localTime) > 0) {
                    std::string logPath = std::string(exePath) + "\\reg_import_debug_" + std::string(timestamp) + ".log";
                    g_logFile.open(logPath, std::ios::app);
                    WriteLog("=== Program started, debug mode enabled ===");
                }
            }
        }
    }

    // 如果启用了debug模式（无论通过哪种方式），初始化日志
    if (g_debugMode && !g_logFile.is_open()) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash != NULL) {
            *lastSlash = '\0';
        }

        CleanOldLogs();

        time_t now = time(0);
        tm localTime;
        if (localtime_s(&localTime, &now) == 0) {
            char timestamp[32];
            if (std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &localTime) > 0) {
                std::string logPath = std::string(exePath) + "\\reg_import_debug_" + std::string(timestamp) + ".log";
                g_logFile.open(logPath, std::ios::app);
                WriteLog("=== Program started, debug mode enabled ===");
            }
        }
    }
    
    // 调试模式下显示控制台窗口
    if (g_debugMode) {
        AllocConsole();
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCout, "CONIN$", "r", stdin);
        SetConsoleOutputCP(65001); // 设置UTF-8编码
        std::cout << "=== Silent Registry Import Tool v" VERSION_STRING " - Debug Mode ===" << std::endl;
    } else {
        // 非调试模式下隐藏控制台窗口
        HWND hwnd = GetConsoleWindow();
        if (hwnd != NULL) {
            ShowWindow(hwnd, SW_HIDE);
        }
    }
    
    WriteLog("Command line arguments: " + cmdLine);
    
    // 解析命令行参数
    if (!cmdLine.empty()) {
        // 支持多个文件路径，用空格分隔
        char* cmdLineCopy = _strdup(cmdLine.c_str());
        char* token = strtok(cmdLineCopy, " ");
        while (token != NULL) {
            std::string pattern = token;
            WriteLog("Processing argument: " + pattern);
            
            // 如果包含通配符，查找匹配的文件
            if (pattern.find('*') != std::string::npos || pattern.find('?') != std::string::npos) {
                FindFiles(pattern, regFiles);
                WriteLog("Wildcard match found " + std::to_string(regFiles.size()) + " files");
            } else {
                // 直接添加文件路径
                regFiles.push_back(pattern);
                WriteLog("Added file: " + pattern);
            }
            token = strtok(NULL, " ");
        }
        free(cmdLineCopy);
    } else {
        // 如果没有参数，使用默认的reg文件
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash != NULL) {
            *lastSlash = '\0';
        }
        
        std::string defaultRegPath = std::string(exePath) + "\\disable_local_network_access.reg";
        regFiles.push_back(defaultRegPath);
        WriteLog("Using default file: " + defaultRegPath);
    }
    
    WriteLog("Total files to import: " + std::to_string(regFiles.size()));

    // 如果是查询模式，执行注册表查询
    if (g_queryMode) {
        WriteLog("Executing registry query...");

        // 打开控制台显示查询结果
        AllocConsole();
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        SetConsoleOutputCP(65001);
        std::cout << "=== Silent Registry Import Tool v" VERSION_STRING " - Registry Query Mode ===" << std::endl;
        std::cout << "Query Path: " << g_queryPath << std::endl;
        std::cout << std::endl;

        QueryRegistry(g_queryPath);

        WriteLog("Registry query completed");
        WriteLog("=== Program finished ===");

        std::cout << std::endl << "=== Query completed ===" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        system("pause > nul");
        FreeConsole();

        if (g_logFile.is_open()) {
            g_logFile.close();
        }
        return 0;
    }

    // 导入所有找到的reg文件
    int successCount = 0;
    for (const auto& regFile : regFiles) {
        if (ImportRegFile(regFile)) {
            successCount++;
        }
    }

    WriteLog("Import completed, success: " + std::to_string(successCount) + ", failed: " + std::to_string(regFiles.size() - static_cast<size_t>(successCount)));
    WriteLog("=== Program finished ===");

    // 调试模式下等待用户按键
    if (g_debugMode) {
        std::cout << "\nPress any key to exit..." << std::endl;
        system("pause > nul");
        FreeConsole();
    }
    
    // 关闭日志文件
    if (g_logFile.is_open()) {
        g_logFile.close();
    }
    
    return (successCount > 0) ? 0 : 1;
}