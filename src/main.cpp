#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "logger.h"
#include "config.h"
#include "audio_player.h"

#ifdef LINUX_SERVICE
#include <csignal>
extern void setup_signal_handlers();
extern bool should_continue_running();
#endif

#ifdef WINDOWS_SERVICE
#include <windows.h>
extern bool is_running_as_service();
extern bool should_continue_running();
extern bool install_service(const std::string& executable_path);
extern bool uninstall_service();
extern void run_service();
extern void signal_shutdown();
#endif

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  -install      Install as system service (Windows only)\n"
              << "  -uninstall    Uninstall system service (Windows only)\n"
              << "  -h, --help    Show this help message\n";
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }

#ifdef WINDOWS_SERVICE
        if (arg == "-install") {
            Config config;
            config.load(Config::get_default_config_path());

            std::string exe_path = argv[0];
            // 获取绝对路径
            char full_path[MAX_PATH];
            GetFullPathNameA(argv[0], MAX_PATH, full_path, nullptr);
            exe_path = full_path;

            Logger::instance().init(config.data().log_file);
            return install_service(exe_path) ? 0 : 1;
        }

        if (arg == "-uninstall") {
            Config config;
            config.load(Config::get_default_config_path());
            Logger::instance().init(config.data().log_file);
            return uninstall_service() ? 0 : 1;
        }
#else
        if (arg == "-install" || arg == "-uninstall") {
            std::cerr << "Service install/uninstall on Linux: use scripts/install-service.sh\n";
            return 1;
        }
#endif
    }

#ifdef WINDOWS_SERVICE
    // Windows: 检查是否作为服务运行
    // 如果不是服务模式，直接运行（便于调试）
#endif

    // 加载配置
    Config config;
    std::string config_path = Config::get_default_config_path();

    if (!config.load(config_path)) {
        // 配置文件不存在，使用默认值
        std::cout << "Config file not found, using defaults\n";
    }

    // 初始化日志
    Logger::instance().init(config.data().log_file);
    Logger::instance().info("Week Sound Service starting...");

    // 设置信号处理
#ifdef LINUX_SERVICE
    setup_signal_handlers();
#endif

    // 初始化音频播放器
    AudioPlayer player(config.data().frequency, config.data().amplitude);

    if (!player.start()) {
        Logger::instance().error("Failed to start audio player: " + player.last_error());
        Logger::instance().info("Will retry every 5 seconds...");

        // 持续重试
        while (should_continue_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (player.retry_initialize()) {
                break;
            }
        }
    }

    // 主循环
    Logger::instance().info("Entering main loop");
    int retry_count = 0;

    while (should_continue_running()) {
        // 每秒检测设备变化
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (player.is_playing()) {
            player.check_device_change();
            retry_count = 0;
        } else {
            // 播放器未运行，尝试重试
            retry_count++;
            if (retry_count >= 5) {
                Logger::instance().warning("Audio playback stopped, retrying...");
                player.retry_initialize();
                retry_count = 0;
            }
        }
    }

    // 清理
    Logger::instance().info("Shutting down...");
    player.stop();
    Logger::instance().info("Service stopped");

    return 0;
}
