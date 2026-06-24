#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger* Logger::instance_ = nullptr;

static std::string escapeJson(const std::string& str) {
    std::string res;
    for (char c : str) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else res += c;
    }
    return res;
}

static std::string eventToJson(const AttendanceEvent& e) {
    char buf[2048];
    snprintf(buf, sizeof(buf),
             R"({"deviceId":"%s","userId":"%s","timestamp":"%s","verifyMode":"%s","status":"%s"})",
             e.deviceId, e.userId, e.timestamp, e.verifyMode, e.status);
    return std::string(buf);
}

void Logger::initialize(const AppConfig& config) {
    if (!instance_) {
        instance_ = new Logger(config);
    }
}

Logger& Logger::getInstance() {
    if (!instance_) {
        std::cerr << "Fatal Error: Logger not initialized" << std::endl;
        exit(1);
    }
    return *instance_;
}

bool Logger::isInitialized() {
    return instance_ != nullptr;
}

Logger::Logger(const AppConfig& config) {
    if (config.logLevel == "debug") minLevel_ = LogLevel::DEBUG_LEVEL;
    else if (config.logLevel == "warn") minLevel_ = LogLevel::WARN_LEVEL;
    else if (config.logLevel == "error") minLevel_ = LogLevel::ERROR_LEVEL;
    else minLevel_ = LogLevel::INFO_LEVEL;

    std::filesystem::path appPath(config.appLog);
    appLogDir_ = appPath.parent_path().string();
    appLogBase_ = appPath.stem().string();

    std::filesystem::path failedPath(config.failedEventsLog);
    failedEventsDir_ = failedPath.parent_path().string();
    failedEventsBase_ = failedPath.stem().string();

    std::filesystem::path attendancePath(config.attendanceLog);
    attendanceDir_ = attendancePath.parent_path().string();
    attendanceBase_ = attendancePath.stem().string();

    if (!appLogDir_.empty()) std::filesystem::create_directories(appLogDir_);
    if (!failedEventsDir_.empty()) std::filesystem::create_directories(failedEventsDir_);
    if (!attendanceDir_.empty()) std::filesystem::create_directories(attendanceDir_);

    currentDate_ = getCurrentDateStr();
    rotateIfNecessary(); 
}

Logger::~Logger() {
    if (appFile_.is_open()) appFile_.close();
    if (failedFile_.is_open()) failedFile_.close();
    if (attendanceFile_.is_open()) attendanceFile_.close();
}

std::string Logger::getCurrentDateStr() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

std::string Logger::getCurrentTimeIso() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string Logger::getLogLevelString(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG_LEVEL: return "debug";
        case LogLevel::INFO_LEVEL: return "info";
        case LogLevel::WARN_LEVEL: return "warn";
        case LogLevel::ERROR_LEVEL: return "error";
        default: return "unknown";
    }
}

void Logger::rotateIfNecessary() {
    std::string today = getCurrentDateStr();
    if (today != currentDate_ || !appFile_.is_open()) {
        currentDate_ = today;

        if (appFile_.is_open()) appFile_.close();
        if (failedFile_.is_open()) failedFile_.close();
        if (attendanceFile_.is_open()) attendanceFile_.close();

        std::string appName = appLogDir_ + "/" + appLogBase_ + "-" + today + ".log";
        appFile_.open(appName, std::ios::app);

        std::string failedName = failedEventsDir_ + "/" + failedEventsBase_ + "-" + today + ".jsonl";
        failedFile_.open(failedName, std::ios::app);

        std::string attendanceName = attendanceDir_ + "/" + attendanceBase_ + "-" + today + ".jsonl";
        attendanceFile_.open(attendanceName, std::ios::app);
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel_) return;

    std::string isoTime = getCurrentTimeIso();
    std::string lvlStr = getLogLevelString(level);

    std::string logLine = R"({"timestamp":")" + isoTime + R"(","level":")" + lvlStr + R"(","message":")" + escapeJson(message) + R"("})";

    if (level == LogLevel::ERROR_LEVEL) {
        std::cerr << "[" << isoTime << "] " << lvlStr << ": " << message << std::endl;
    } else {
        std::cout << "[" << isoTime << "] " << lvlStr << ": " << message << std::endl;
    }

    std::lock_guard<std::mutex> lock(appMutex_);
    rotateIfNecessary();
    if (appFile_.is_open()) {
        appFile_ << logLine << "\n";
        appFile_.flush();
    }
}

void Logger::debug(const std::string& message) { log(LogLevel::DEBUG_LEVEL, message); }
void Logger::info(const std::string& message)  { log(LogLevel::INFO_LEVEL, message); }
void Logger::warn(const std::string& message)  { log(LogLevel::WARN_LEVEL, message); }
void Logger::error(const std::string& message) { log(LogLevel::ERROR_LEVEL, message); }

void Logger::logFailedEvent(const AttendanceEvent& event, const std::string& errorMsg) {
    std::string isoTime = getCurrentTimeIso();
    std::string eventJson = eventToJson(event);

    std::string logLine = R"({"type":"failed-event","failedAt":")" + isoTime + R"(","error":")" + escapeJson(errorMsg) + R"(","event":)" + eventJson + R"(})";

    std::lock_guard<std::mutex> lock(failedMutex_);
    rotateIfNecessary();
    if (failedFile_.is_open()) {
        failedFile_ << logLine << "\n";
        failedFile_.flush();
    }
}

void Logger::logAttendance(const AttendanceEvent& event) {
    std::string isoTime = getCurrentTimeIso();
    
    std::string logLine = R"({"type":"attendance-entry","loggedAt":")" + isoTime + R"(","deviceId":")" + event.deviceId + 
                          R"(","userId":")" + event.userId + R"(","timestamp":")" + event.timestamp + 
                          R"(","verifyMode":")" + event.verifyMode + R"(","status":")" + event.status + R"("})";

    std::lock_guard<std::mutex> lock(attendanceMutex_);
    rotateIfNecessary();
    if (attendanceFile_.is_open()) {
        attendanceFile_ << logLine << "\n";
        attendanceFile_.flush();
    }
}
