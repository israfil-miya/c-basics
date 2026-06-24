#pragma once

#include "../proto/parser.h"
#include <string>

struct ApiConfig {
  std::string cloudApiUrl;
  std::string cloudApiAuthHeader;
  std::string cloudApiAuthValue;
  long cloudApiTimeoutMs = 5000;
};

struct ApiResponse {
  bool success;
  long statusCode;
  std::string errorMsg;
};

class CloudApiClient {
public:
  explicit CloudApiClient(const ApiConfig &config);
  ~CloudApiClient();

  // Prevent copying
  CloudApiClient(const CloudApiClient &) = delete;
  CloudApiClient &operator=(const CloudApiClient &) = delete;

  ApiResponse sendEvent(const AttendanceEvent &event);

private:
  ApiConfig config_;
  void *curl_;         // hide CURL* to avoid bringing in curl.h for everyone
  void *curl_headers_; // hide curl_slist*
};
