#include "config.h"
#include <fstream>
#include <cstdlib>
#include <iostream>

static void loadEnvFile() {
    std::ifstream file(".env");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        size_t eqIndex = line.find('=');
        if (eqIndex == std::string::npos) continue;

        std::string key = line.substr(0, eqIndex);
        std::string value = line.substr(eqIndex + 1);

        // Trim key
        key.erase(key.find_last_not_of(" \t") + 1);

        // Trim value
        size_t valStart = value.find_first_not_of(" \t\r\n");
        if (valStart == std::string::npos) {
            value = "";
        } else {
            value = value.substr(valStart);
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
        }

        // Remove surrounding quotes if present
        if (value.size() >= 2 && 
            ((value.front() == '"' && value.back() == '"') || 
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        // Set env variable if not already set in OS environment
        if (std::getenv(key.c_str()) == nullptr) {
            setenv(key.c_str(), value.c_str(), 0);
        }
    }
}

AppConfig AppConfig::load() {
    loadEnvFile();
    AppConfig config;

    const char* portStr = std::getenv("ADMS_PORT");
    if (portStr) {
        try { config.admsPort = std::stoi(portStr); } 
        catch (...) { config.admsPort = 4370; }
    }

    const char* timeoutStr = std::getenv("CLOUD_API_TIMEOUT");
    if (timeoutStr) {
        try { config.cloudApiTimeoutMs = std::stoi(timeoutStr); } 
        catch (...) { config.cloudApiTimeoutMs = 10000; }
    }

    const char* urlStr = std::getenv("CLOUD_API_URL");
    if (urlStr) config.cloudApiUrl = urlStr;

    const char* authHeadStr = std::getenv("CLOUD_API_AUTH_HEADER");
    if (authHeadStr) config.cloudApiAuthHeader = authHeadStr;

    const char* authValStr = std::getenv("CLOUD_API_AUTH_VALUE");
    if (authValStr) config.cloudApiAuthValue = authValStr;

    if ((!config.cloudApiAuthHeader.empty() && config.cloudApiAuthValue.empty()) ||
        (config.cloudApiAuthHeader.empty() && !config.cloudApiAuthValue.empty())) {
        std::cerr << "Fatal Error: CLOUD_API_AUTH_HEADER and CLOUD_API_AUTH_VALUE must be set together, or neither" << std::endl;
        exit(1);
    }

    const char* logLevelStr = std::getenv("LOG_LEVEL");
    if (logLevelStr) {
        config.logLevel = logLevelStr;
        if (config.logLevel != "debug" && config.logLevel != "info" && 
            config.logLevel != "warn" && config.logLevel != "error") {
            std::cerr << "Fatal Error: Invalid LOG_LEVEL. Must be debug, info, warn, or error." << std::endl;
            exit(1);
        }
    }

    const char* failedLogStr = std::getenv("FAILED_EVENTS_LOG");
    if (failedLogStr) config.failedEventsLog = failedLogStr;

    const char* appLogStr = std::getenv("APP_LOG");
    if (appLogStr) config.appLog = appLogStr;

    const char* attendanceLogStr = std::getenv("ATTENDANCE_LOG");
    if (attendanceLogStr) config.attendanceLog = attendanceLogStr;

    return config;
}
