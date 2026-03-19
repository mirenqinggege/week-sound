#ifdef WINDOWS_SERVICE

#include <windows.h>
#include <string>
#include <atomic>

#include "logger.h"

static SERVICE_STATUS g_service_status = {};
static SERVICE_STATUS_HANDLE g_status_handle = nullptr;
static std::atomic<bool> g_running(true);
static std::atomic<bool> g_is_service(false);

// 前向声明
void service_main(DWORD argc, LPSTR* argv);
void service_ctrl_handler(DWORD ctrl_code);
void report_service_status(DWORD current_state, DWORD win32_exit_code, DWORD wait_hint);

// 服务入口点注册
SERVICE_TABLE_ENTRY service_table[] = {
    {(LPSTR)"WeekSoundService", (LPSERVICE_MAIN_FUNCTION)service_main},
    {nullptr, nullptr}
};

bool is_running_as_service() {
    return g_is_service.load();
}

bool should_continue_running() {
    return g_running.load();
}

void signal_shutdown() {
    g_running = false;
}

bool install_service(const std::string& executable_path) {
    SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        Logger::instance().error("Failed to open Service Control Manager");
        return false;
    }

    SC_HANDLE service = CreateServiceA(
        scm,
        "WeekSoundService",
        "USB Soundcard Anti-Sleep Service",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        executable_path.c_str(),
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!service) {
        DWORD error = GetLastError();
        CloseServiceHandle(scm);
        if (error == ERROR_SERVICE_EXISTS) {
            Logger::instance().warning("Service already exists");
        } else {
            Logger::instance().error("Failed to create service, error: " + std::to_string(error));
        }
        return false;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    Logger::instance().info("Service installed successfully");
    return true;
}

bool uninstall_service() {
    SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        Logger::instance().error("Failed to open Service Control Manager");
        return false;
    }

    SC_HANDLE service = OpenServiceA(scm, "WeekSoundService", SERVICE_ALL_ACCESS);
    if (!service) {
        CloseServiceHandle(scm);
        Logger::instance().error("Service not found");
        return false;
    }

    // 先停止服务
    SERVICE_STATUS status;
    ControlService(service, SERVICE_CONTROL_STOP, &status);

    bool result = DeleteService(service);

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    if (result) {
        Logger::instance().info("Service uninstalled successfully");
    } else {
        Logger::instance().error("Failed to delete service");
    }

    return result;
}

void run_service() {
    g_is_service = true;

    if (!StartServiceCtrlDispatcherA(service_table)) {
        Logger::instance().error("StartServiceCtrlDispatcher failed: " + std::to_string(GetLastError()));
        g_running = false;
    }
}

void service_main(DWORD argc, LPSTR* argv) {
    (void)argc;
    (void)argv;

    g_status_handle = RegisterServiceCtrlHandlerA("WeekSoundService", service_ctrl_handler);
    if (!g_status_handle) {
        Logger::instance().error("RegisterServiceCtrlHandler failed");
        return;
    }

    g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    report_service_status(SERVICE_START_PENDING, NO_ERROR, 3000);

    // 服务启动完成
    report_service_status(SERVICE_RUNNING, NO_ERROR, 0);
    Logger::instance().info("Service started");
}

void service_ctrl_handler(DWORD ctrl_code) {
    switch (ctrl_code) {
        case SERVICE_CONTROL_STOP:
            report_service_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
            g_running = false;
            report_service_status(SERVICE_STOPPED, NO_ERROR, 0);
            break;

        case SERVICE_CONTROL_PAUSE:
            report_service_status(SERVICE_PAUSE_PENDING, NO_ERROR, 0);
            // 暂停逻辑可以在这里实现
            report_service_status(SERVICE_PAUSED, NO_ERROR, 0);
            break;

        case SERVICE_CONTROL_CONTINUE:
            report_service_status(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
            // 恢复逻辑可以在这里实现
            report_service_status(SERVICE_RUNNING, NO_ERROR, 0);
            break;

        default:
            break;
    }
}

void report_service_status(DWORD current_state, DWORD win32_exit_code, DWORD wait_hint) {
    g_service_status.dwCurrentState = current_state;
    g_service_status.dwWin32ExitCode = win32_exit_code;
    g_service_status.dwWaitHint = wait_hint;
    g_service_status.dwCheckPoint = (current_state == SERVICE_RUNNING || current_state == SERVICE_STOPPED) ? 0 : 1;

    SetServiceStatus(g_status_handle, &g_service_status);
}

#endif // WINDOWS_SERVICE
