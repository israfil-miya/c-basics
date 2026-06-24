#pragma once

#include "../proto/parser.h"
#include "../config.h"
#include <string>
#include <mutex>
#include <fstream>
#include <filesystem>

enum class LogLevel {
    DEBUG_LEVEL,
    INFO_LEVEL,
    WARN_LEVEL,
    ERROR_LEVEL
};

class Logger {
public:
    static void initialize(const AppConfig& config);
    static Logger& getInstance();
    static bool isInitialized();

    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    void logFailedEvent(const AttendanceEvent& event, const std::string& errorMsg);
    void logAttendance(const AttendanceEvent& event);

private:
    Logger(const AppConfig& config);
    ~Logger();

    void log(LogLevel level, const std::string& message);
    void rotateIfNecessary();

    std::string getLogLevelString(LogLevel level);
    std::string getCurrentDateStr();
    std::string getCurrentTimeIso();
    
    LogLevel minLevel_;
    std::string appLogDir_;
    std::string appLogBase_;
    std::string failedEventsDir_;
    std::string failedEventsBase_;
    std::string attendanceDir_;
    std::string attendanceBase_;
    
    std::string currentDate_;

    std::mutex appMutex_;
    std::ofstream appFile_;

    std::mutex failedMutex_;
    std::ofstream failedFile_;

    std::mutex attendanceMutex_;
    std::ofstream attendanceFile_;

    static Logger* instance_;
};
