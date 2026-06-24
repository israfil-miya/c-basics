#include "poster.h"
#include "../log/logger.h"
#include <cstdio>
#include <curl/curl.h>

static std::string escapeJson(const char *str) {
  std::string res;
  while (*str) {
    if (*str == '"')
      res += "\\\"";
    else if (*str == '\\')
      res += "\\\\";
    else if (*str == '\b')
      res += "\\b";
    else if (*str == '\f')
      res += "\\f";
    else if (*str == '\n')
      res += "\\n";
    else if (*str == '\r')
      res += "\\r";
    else if (*str == '\t')
      res += "\\t";
    else
      res += *str;
    str++;
  }
  return res;
}

static std::string eventToJson(const AttendanceEvent &e) {
  char buf[2048];
  snprintf(
      buf, sizeof(buf),
      R"({"deviceId":"%s","userId":"%s","timestamp":"%s","rawTimestamp":"%s","verifyMode":"%s","status":"%s","sourceIp":"%s","receivedAt":"%s"})",
      escapeJson(e.deviceId).c_str(), escapeJson(e.userId).c_str(),
      escapeJson(e.timestamp).c_str(), escapeJson(e.rawTimestamp).c_str(),
      escapeJson(e.verifyMode).c_str(), escapeJson(e.status).c_str(),
      escapeJson(e.sourceIp).c_str(), escapeJson(e.receivedAt).c_str());
  return std::string(buf);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  auto *str = static_cast<std::string *>(userp);
  size_t realsize = size * nmemb;
  str->append(static_cast<char *>(contents), realsize);
  return realsize;
}

CloudApiClient::CloudApiClient(const ApiConfig &config)
    : config_(config), curl_(nullptr), curl_headers_(nullptr) {
  CURL *curl = curl_easy_init();
  curl_ = curl;
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, config_.cloudApiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config_.cloudApiTimeoutMs);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers =
        curl_slist_append(headers, "User-Agent: ZKTeco-ADMS-Listener/1.0");

    if (!config_.cloudApiAuthHeader.empty() &&
        !config_.cloudApiAuthValue.empty()) {
      std::string auth =
          config_.cloudApiAuthHeader + ": " + config_.cloudApiAuthValue;
      headers = curl_slist_append(headers, auth.c_str());
    }

    curl_headers_ = headers;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // --- THE CRITICAL OPTIMIZATION ---
    // By setting these and reusing the CURL* handle, libcurl automatically
    // maintains a persistent connection pool to the destination host!
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
  }
}

CloudApiClient::~CloudApiClient() {
  if (curl_headers_) {
    curl_slist_free_all(static_cast<struct curl_slist *>(curl_headers_));
  }
  if (curl_) {
    curl_easy_cleanup(static_cast<CURL *>(curl_));
  }
}

ApiResponse CloudApiClient::sendEvent(const AttendanceEvent &event) {
  ApiResponse res;
  res.success = false;
  res.statusCode = 0;

  CURL *curl = static_cast<CURL *>(curl_);
  if (!curl) {
    res.errorMsg = "CURL not initialized";
    return res;
  }

  std::string payload = eventToJson(event);
  std::string responseBody;

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                   static_cast<long>(payload.length()));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

  CURLcode res_code = curl_easy_perform(curl);

  if (res_code != CURLE_OK) {
    res.errorMsg = curl_easy_strerror(res_code);
    if (Logger::isInitialized()) {
        Logger::getInstance().error("API POST failed - network error: " + res.errorMsg);
        Logger::getInstance().logFailedEvent(event, res.errorMsg);
    }
    return res;
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.statusCode);

  if (res.statusCode >= 200 && res.statusCode < 300) {
    res.success = true;
    // if (Logger::isInitialized()) Logger::getInstance().info("API POST success for user " + std::string(event.userId));
  } else {
    res.errorMsg = "HTTP " + std::to_string(res.statusCode) + ": " +
                   responseBody.substr(0, 200);
    if (Logger::isInitialized()) {
        Logger::getInstance().error("API POST failed - non-2xx response: " + res.errorMsg);
        Logger::getInstance().logFailedEvent(event, res.errorMsg);
    }
  }

  return res;
}
