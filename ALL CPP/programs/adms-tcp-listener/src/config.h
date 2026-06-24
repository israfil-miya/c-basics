#pragma once

#include <string>

struct AppConfig {
    int admsPort = 4370;
    std::string cloudApiUrl;
    std::string cloudApiAuthHeader;
    std::string cloudApiAuthValue;
    int cloudApiTimeoutMs = 10000;
    
    std::string logLevel = "info";
    std::string failedEventsLog = "./logs/failed-events.jsonl";
    std::string appLog = "./logs/app.log";
    std::string attendanceLog = "./logs/attendance.jsonl";

    static AppConfig load();
};
