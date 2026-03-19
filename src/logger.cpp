#include "logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& log_file) {
    std::lock_guard<std::mutex> lock(mutex_);

    log_file_.open(log_file, std::ios::app);
    if (log_file_.is_open()) {
        initialized_ = true;
    } else {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
    }
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERR, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string log_line = "[" + get_timestamp() + "] [" + level_to_string(level) + "] " + message;

    if (initialized_ && log_file_.is_open()) {
        log_file_ << log_line << std::endl;
        log_file_.flush();
    }

    // 同时输出到 stderr（仅 WARNING 和 ERROR）
    if (level != LogLevel::INFO) {
        std::cerr << log_line << std::endl;
    }
}

std::string Logger::get_timestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERR: return "ERROR";
        default: return "UNKNOWN";
    }
}
