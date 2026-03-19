#ifndef WEEK_SOUND_LOGGER_H
#define WEEK_SOUND_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <ctime>

enum class LogLevel {
    INFO,
    WARNING,
    ERR  // 避免与 Windows ERROR 宏冲突
};

class Logger {
public:
    static Logger& instance();

    void init(const std::string& log_file);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    ~Logger() = default;

    void log(LogLevel level, const std::string& message);
    std::string get_timestamp();
    std::string level_to_string(LogLevel level);

    std::ofstream log_file_;
    std::mutex mutex_;
    bool initialized_ = false;
};

#endif // WEEK_SOUND_LOGGER_H
