#if !defined(LOG_MANAGER_HPP)
#define LOG_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <mutex>

class LogManager {
public:
    enum LogLevel {
        INFO,
        WARNING,
        ERR,
        DEBUG
    };

    // Singleton pattern
    static LogManager& getInstance() {
        static LogManager instance;
        return instance;
    }

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (filename.find(".log") == std::string::npos) {
            std::cerr << "Invalid log file extension: " << filename << std::endl;
            return;
        }

        if (logFile_.is_open()) {
            logFile_.close();
        }
        logFile_.open(filename, std::ios::app);
        if (!logFile_) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        logLevel_ = level;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (level < logLevel_) return; // Skip logs below the current log level

        std::string levelStr = getLogLevelString(level);
        std::string timestamp = getCurrentTime();

        std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

        // Log to file if available
        if (logFile_) {
            logFile_ << logMessage << std::endl;
        }

        // Always log to console
        std::cout << logMessage << std::endl;
    }

private:
    LogManager() : logLevel_(INFO) {}
    ~LogManager() {
        if (logFile_) {
            logFile_.close();
        }
    }

    LogManager(const LogManager&) = delete;            // Disable copy constructor
    LogManager& operator=(const LogManager&) = delete; // Disable assignment operator

    std::string getLogLevelString(LogLevel level) {
        switch (level) {
            case INFO:    return "INFO";
            case WARNING: return "WARNING";
            case ERR:     return "ERROR";
            case DEBUG:   return "DEBUG";
            default:      return "UNKNOWN";
        }
    }

    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        char buf[80];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buf);
    }

    std::ofstream logFile_;
    LogLevel logLevel_;
    std::mutex mutex_;
};



#endif // LOG_MANAGER_HPP
