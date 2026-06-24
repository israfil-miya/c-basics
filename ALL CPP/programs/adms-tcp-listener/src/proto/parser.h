#pragma once

#include <cctype>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

struct AttendanceEvent {
  char deviceId[64];
  char userId[32];
  char timestamp[32]; // ISO 8601
  char rawTimestamp[32];
  char verifyMode[16];
  char status[16];
  char sourceIp[INET_ADDRSTRLEN];
  char receivedAt[32];
};

enum class ParseStatus { Success, NotAttendanceData, Error };

struct CaseInsensitiveHash {
  size_t operator()(std::string_view sv) const {
    size_t h = 0;
    for (char c : sv) {
      h = h * 31 + std::tolower(static_cast<unsigned char>(c));
    }
    return h;
  }
};

struct CaseInsensitiveEqual {
  bool operator()(std::string_view s1, std::string_view s2) const {
    if (s1.size() != s2.size())
      return false;
    for (size_t i = 0; i < s1.size(); ++i) {
      if (std::tolower(static_cast<unsigned char>(s1[i])) !=
          std::tolower(static_cast<unsigned char>(s2[i]))) {
        return false;
      }
    }
    return true;
  }
};

class AdmsParser {
public:
  static ParseStatus parse(std::span<const char> raw, const char *sourceIp,
                           AttendanceEvent &out);
  static std::vector<AttendanceEvent> parseMultiple(std::span<const char> raw,
                                                    const char *sourceIp);

private:
  using StringMap =
      std::unordered_map<std::string_view, std::string_view,
                         CaseInsensitiveHash, CaseInsensitiveEqual>;

  static StringMap parseQueryString(std::string_view data);
  static StringMap parseKeyValuePairs(std::string_view data);

  static std::string extractDeviceId(std::string_view data,
                                     const StringMap &queryParams,
                                     const StringMap &kvPairs);
  static std::string_view extractUserId(const StringMap &kvPairs);
  static std::string parseTimestamp(std::string_view rawTimestamp);
  static std::string_view extractTimestamp(const StringMap &kvPairs);
  static bool isAttendanceData(std::string_view data,
                               const StringMap &queryParams);

  static std::string_view getVerifyMode(std::string_view rawVerify);
  static std::string_view getStatusType(std::string_view rawStatus);
};
