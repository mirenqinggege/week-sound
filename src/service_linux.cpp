#include "logger.h"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#ifdef LINUX_SERVICE

static std::atomic<bool> g_running(true);

static void signal_handler(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        Logger::instance().info("Received signal " + std::to_string(signal) + ", shutting down...");
        g_running = false;
    }
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
}

bool should_continue_running() {
    return g_running.load();
}

void wait_for_shutdown() {
    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

#endif // LINUX_SERVICE
