#ifdef WINDOWS_SERVICE

#include <windows.h>
#include <string>
#include <atomic>

#include "logger.h"

static std::atomic<bool> g_running(true);

// Windows 控制台信号处理
static BOOL WINAPI console_handler(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT || ctrl_type == CTRL_CLOSE_EVENT) {
        Logger::instance().info("Received shutdown signal");
        g_running = false;
        return TRUE;
    }
    return FALSE;
}

bool should_continue_running() {
    return g_running.load();
}

bool install_service(const std::string& executable_path) {
    // 使用注册表实现用户登录时自动启动（非系统服务）
    // 原因：Windows 系统服务以 SYSTEM 账户运行，无法访问音频设备

    HKEY hkey;
    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hkey
    );

    if (result != ERROR_SUCCESS) {
        Logger::instance().error("Failed to open registry key, error: " + std::to_string(result));
        return false;
    }

    // 设置自动启动项
    result = RegSetValueExA(
        hkey,
        "WeekSound",
        0,
        REG_SZ,
        (const BYTE*)executable_path.c_str(),
        static_cast<DWORD>(executable_path.length() + 1)
    );

    RegCloseKey(hkey);

    if (result != ERROR_SUCCESS) {
        Logger::instance().error("Failed to set registry value, error: " + std::to_string(result));
        return false;
    }

    Logger::instance().info("Auto-start entry added to registry (current user)");
    Logger::instance().info("The program will start automatically when you log in");
    return true;
}

bool uninstall_service() {
    // 从注册表删除自动启动项
    HKEY hkey;
    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hkey
    );

    if (result != ERROR_SUCCESS) {
        Logger::instance().error("Failed to open registry key, error: " + std::to_string(result));
        return false;
    }

    result = RegDeleteValueA(hkey, "WeekSound");
    RegCloseKey(hkey);

    if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND) {
        Logger::instance().error("Failed to delete registry value, error: " + std::to_string(result));
        return false;
    }

    Logger::instance().info("Auto-start entry removed from registry");
    return true;
}

// 初始化控制台信号处理
void setup_console_handler() {
    SetConsoleCtrlHandler(console_handler, TRUE);
}

#endif // WINDOWS_SERVICE
