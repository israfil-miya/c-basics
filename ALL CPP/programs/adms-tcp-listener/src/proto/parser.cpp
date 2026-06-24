#include "parser.h"
#include <algorithm>
#include <chrono>
#include <cstdio>

namespace {

int hexCharToInt(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

std::string urlDecode(std::string_view str) {
  std::string ret;
  ret.reserve(str.length());
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '%') {
      if (i + 2 < str.length()) {
        int h1 = hexCharToInt(str[i + 1]);
        int h2 = hexCharToInt(str[i + 2]);
        if (h1 != -1 && h2 != -1) {
          ret += static_cast<char>((h1 << 4) | h2);
          i += 2;
        } else {
          ret += str[i];
        }
      } else {
        ret += str[i];
      }
    } else if (str[i] == '+') {
      ret += ' ';
    } else {
      ret += str[i];
    }
  }
  return ret;
}

std::string_view trimView(std::string_view s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string_view::npos)
    return {};
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

bool containsIgnoreCase(std::string_view haystack, std::string_view needle) {
  auto it =
      std::search(haystack.begin(), haystack.end(), needle.begin(),
                  needle.end(), [](char ch1, char ch2) {
                    return std::toupper(static_cast<unsigned char>(ch1)) ==
                           std::toupper(static_cast<unsigned char>(ch2));
                  });
  return it != haystack.end();
}

std::string getCurrentTimeIso() {
  auto now = std::chrono::system_clock::now();
  auto timeT = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  gmtime_s(&tm, &timeT);
#else
  gmtime_r(&timeT, &tm);
#endif
  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
  return std::string(buf);
}

static void copyToFixed(char *dest, size_t destSize, std::string_view src) {
  if (destSize == 0)
    return;
  size_t copyLen = std::min(src.size(), destSize - 1);
  std::copy_n(src.data(), copyLen, dest);
  dest[copyLen] = '\0';
}

} // namespace

AdmsParser::StringMap AdmsParser::parseQueryString(std::string_view data) {
  StringMap params;
  size_t start = data.find_first_of("?&");
  if (start != std::string_view::npos) {
    size_t end = data.find_first_of(" #\r\n", start);
    std::string_view query = data.substr(
        start + 1, end == std::string_view::npos ? std::string_view::npos
                                                 : end - (start + 1));

    size_t pos = 0;
    while (pos < query.length()) {
      size_t amp = query.find('&', pos);
      std::string_view pair = query.substr(pos, amp == std::string_view::npos
                                                    ? std::string_view::npos
                                                    : amp - pos);

      size_t eq = pair.find('=');
      if (eq != std::string_view::npos) {
        // Key and Value are both string_views pointing to the raw buffer
        params[pair.substr(0, eq)] = pair.substr(eq + 1);
      } else if (!pair.empty()) {
        params[pair] = std::string_view{};
      }
      if (amp == std::string_view::npos)
        break;
      pos = amp + 1;
    }
  }
  return params;
}

AdmsParser::StringMap AdmsParser::parseKeyValuePairs(std::string_view data) {
  StringMap result;
  size_t lineStart = 0;

  while (lineStart < data.length()) {
    size_t lineEnd = data.find_first_of("\r\n", lineStart);
    std::string_view line = data.substr(
        lineStart, lineEnd == std::string_view::npos ? std::string_view::npos
                                                     : lineEnd - lineStart);

    if (!line.empty() && !line.starts_with("POST ") &&
        !line.starts_with("GET ") && !line.starts_with("HTTP/")) {
      bool isHttpHeader = (line.find(": ") != std::string_view::npos &&
                           line.find('=') == std::string_view::npos);
      if (!isHttpHeader && !trimView(line).empty()) {
        // Tab separated ATTLOG format
        if (line.find('\t') != std::string_view::npos) {
          std::vector<std::string_view> parts;
          size_t p = 0;
          while (p < line.length()) {
            size_t t = line.find('\t', p);
            parts.push_back(line.substr(p, t == std::string_view::npos
                                               ? std::string_view::npos
                                               : t - p));
            if (t == std::string_view::npos)
              break;
            p = t + 1;
          }

          auto isDigits = [](std::string_view s) {
            return !s.empty() &&
                   std::all_of(s.begin(), s.end(),
                               [](unsigned char c) { return std::isdigit(c); });
          };

          if (parts.size() >= 4 && isDigits(trimView(parts[0]))) {
            // Avoid string allocation, use static keys or string_views mapped
            // to the data buffer
            result["PIN"] = trimView(parts[0]);
            result["TIME"] = trimView(parts[1]);
            result["STATUS"] = trimView(parts[2]);
            result["VERIFY"] = trimView(parts[3]);
            if (parts.size() > 4)
              result["WORKCODE"] = trimView(parts[4]);
            goto next_line;
          } else {
            for (auto part : parts) {
              size_t sep = part.find_first_of(":=");
              if (sep != std::string_view::npos) {
                result[part.substr(0, sep)] = trimView(part.substr(sep + 1));
              }
            }
            goto next_line;
          }
        }

        // Comma separated
        if (line.find(',') != std::string_view::npos) {
          size_t p = 0;
          while (p < line.length()) {
            size_t t = line.find(',', p);
            std::string_view part = line.substr(p, t == std::string_view::npos
                                                       ? std::string_view::npos
                                                       : t - p);
            size_t sep = part.find_first_of(":=");
            if (sep != std::string_view::npos) {
              result[part.substr(0, sep)] = trimView(part.substr(sep + 1));
            }
            if (t == std::string_view::npos)
              break;
            p = t + 1;
          }
          goto next_line;
        }

        // Space separated with =
        if (line.find(' ') != std::string_view::npos &&
            line.find('=') != std::string_view::npos) {
          size_t p = 0;
          while (p < line.length()) {
            size_t t = line.find(' ', p);
            std::string_view part = line.substr(p, t == std::string_view::npos
                                                       ? std::string_view::npos
                                                       : t - p);
            if (!part.empty()) {
              size_t sep = part.find_first_of(":=");
              if (sep != std::string_view::npos) {
                result[part.substr(0, sep)] = trimView(part.substr(sep + 1));
              }
            }
            if (t == std::string_view::npos)
              break;
            p = t + 1;
          }
          goto next_line;
        }

        // Single key=value
        {
          size_t sep = line.find_first_of(":=");
          if (sep != std::string_view::npos) {
            result[line.substr(0, sep)] = trimView(line.substr(sep + 1));
          }
        }
      }
    }
  next_line:
    if (lineEnd == std::string_view::npos)
      break;
    lineStart = lineEnd + 1;
    if (lineStart < data.length() && data[lineEnd] == '\r' &&
        data[lineStart] == '\n') {
      lineStart++;
    }
  }
  return result;
}

std::string AdmsParser::extractDeviceId(std::string_view data,
                                        const StringMap &queryParams,
                                        const StringMap &kvPairs) {
  if (auto it = queryParams.find("SN"); it != queryParams.end())
    return urlDecode(it->second);

  for (std::string_view key : {"SN", "SERIALNUMBER", "DEVICE"}) {
    if (auto it = kvPairs.find(key); it != kvPairs.end())
      return std::string(it->second);
  }

  size_t pos = data.find("SN=");
  if (pos == std::string_view::npos)
    pos = data.find("SN:");
  if (pos != std::string_view::npos) {
    pos += 3;
    size_t end = data.find_first_of(" \t\r\n&,", pos);
    return std::string(data.substr(pos, end == std::string_view::npos
                                            ? std::string_view::npos
                                            : end - pos));
  }
  return "UNKNOWN";
}

std::string_view AdmsParser::extractUserId(const StringMap &kvPairs) {
  for (std::string_view key : {"PIN", "USERID", "USER", "ENROLLID", "ID"}) {
    if (auto it = kvPairs.find(key); it != kvPairs.end())
      return it->second;
  }
  return {};
}

std::string AdmsParser::parseTimestamp(std::string_view rawTimestamp) {
  std::string s{trimView(rawTimestamp)};
  if (s.empty())
    return s;

  int y, M, d, h, m, sec;
  char sep1, sep2, sep3;

  // YYYY-MM-DD HH:MM:SS
  if (sscanf(s.c_str(), "%d%c%d%c%d%c%d:%d:%d", &y, &sep1, &M, &sep2, &d, &sep3,
             &h, &m, &sec) == 9) {
    if ((sep1 == '-' || sep1 == '/') && (sep2 == '-' || sep2 == '/') &&
        (sep3 == ' ' || sep3 == 'T')) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d", y, M, d, h, m,
               sec);
      return buf;
    }
  }

  // Compact YYYYMMDDHHMMSS
  if (s.length() >= 14 &&
      std::all_of(s.begin(), s.begin() + 14,
                  [](unsigned char c) { return std::isdigit(c); })) {
    if (sscanf(s.substr(0, 14).c_str(), "%4d%2d%2d%2d%2d%2d", &y, &M, &d, &h,
               &m, &sec) == 6) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d", y, M, d, h, m,
               sec);
      return buf;
    }
  }

  for (char &c : s)
    if (c == ' ')
      c = 'T';
  if (s.back() == 'Z' || s.back() == 'z')
    s.pop_back();
  return s;
}

std::string_view AdmsParser::extractTimestamp(const StringMap &kvPairs) {
  for (std::string_view key :
       {"TIME", "DATETIME", "CHECKTIME", "ATTTIME", "TIMESTAMP"}) {
    if (auto it = kvPairs.find(key); it != kvPairs.end())
      return it->second;
  }
  return {};
}

bool AdmsParser::isAttendanceData(std::string_view data,
                                  const StringMap &queryParams) {
  if (auto it = queryParams.find("TABLE"); it != queryParams.end()) {
    if (containsIgnoreCase(it->second, "ATTLOG"))
      return true;
  }

  if (containsIgnoreCase(data, "ATTLOG") ||
      containsIgnoreCase(data, "TABLE=ATTLOG") ||
      containsIgnoreCase(data, "CMD=ATT") ||
      containsIgnoreCase(data, "CMD:ATT") ||
      containsIgnoreCase(data, "CHECKTIME=") ||
      containsIgnoreCase(data, "CHECKTIME:")) {
    return true;
  }

  if (containsIgnoreCase(data, "PIN=") || containsIgnoreCase(data, "PIN:")) {
    if (containsIgnoreCase(data, "TIME=") ||
        containsIgnoreCase(data, "TIME:")) {
      return true;
    }
  }
  return false;
}

std::string_view AdmsParser::getVerifyMode(std::string_view rawVerify) {
  static const StringMap modes = {
      {"0", "password"},     {"1", "fingerprint"}, {"2", "card"},
      {"3", "password"},     {"4", "fingerprint"}, {"6", "face"},
      {"10", "face"},        {"11", "face"},       {"15", "face"},
      {"FP", "fingerprint"}, {"PW", "password"},   {"RF", "card"},
      {"FACE", "face"}};
  if (auto it = modes.find(rawVerify); it != modes.end())
    return it->second;
  return rawVerify;
}

std::string_view AdmsParser::getStatusType(std::string_view rawStatus) {
  static const StringMap statuses = {
      {"0", "check-in"},     {"1", "check-out"},   {"2", "break-out"},
      {"3", "break-in"},     {"4", "overtime-in"}, {"5", "overtime-out"},
      {"255", "unspecified"}};
  if (auto it = statuses.find(rawStatus); it != statuses.end())
    return it->second;
  return rawStatus;
}

ParseStatus AdmsParser::parse(std::span<const char> raw, const char *sourceIp,
                              AttendanceEvent &out) {
  std::string_view data(raw.data(), raw.size());
  std::string_view normalized = trimView(data);
  if (normalized.empty())
    return ParseStatus::Error;

  auto queryParams = parseQueryString(normalized);
  auto kvPairs = parseKeyValuePairs(normalized);

  if (!isAttendanceData(normalized, queryParams)) {
    return ParseStatus::NotAttendanceData;
  }

  std::string_view userId = extractUserId(kvPairs);
  if (userId.empty())
    return ParseStatus::Error;

  std::string_view rawTimestamp = extractTimestamp(kvPairs);
  if (rawTimestamp.empty())
    return ParseStatus::Error;

  copyToFixed(out.deviceId, sizeof(out.deviceId),
              extractDeviceId(normalized, queryParams, kvPairs));
  copyToFixed(out.userId, sizeof(out.userId), userId);
  copyToFixed(out.rawTimestamp, sizeof(out.rawTimestamp), rawTimestamp);
  copyToFixed(out.timestamp, sizeof(out.timestamp),
              parseTimestamp(rawTimestamp));

  std::string_view verifyModeStr = "0";
  if (kvPairs.count("VERIFY"))
    verifyModeStr = kvPairs.at("VERIFY");
  else if (kvPairs.count("VERIFYMODE"))
    verifyModeStr = kvPairs.at("VERIFYMODE");
  else if (kvPairs.count("VERIFIED"))
    verifyModeStr = kvPairs.at("VERIFIED");
  copyToFixed(out.verifyMode, sizeof(out.verifyMode),
              getVerifyMode(verifyModeStr));

  std::string_view statusStr = "0";
  if (kvPairs.count("STATUS"))
    statusStr = kvPairs.at("STATUS");
  else if (kvPairs.count("INOUTMODE"))
    statusStr = kvPairs.at("INOUTMODE");
  else if (kvPairs.count("WORKCODE"))
    statusStr = kvPairs.at("WORKCODE");
  copyToFixed(out.status, sizeof(out.status), getStatusType(statusStr));

  copyToFixed(out.sourceIp, sizeof(out.sourceIp), sourceIp ? sourceIp : "");
  copyToFixed(out.receivedAt, sizeof(out.receivedAt), getCurrentTimeIso());

  return ParseStatus::Success;
}

std::vector<AttendanceEvent>
AdmsParser::parseMultiple(std::span<const char> raw, const char *sourceIp) {
  std::vector<AttendanceEvent> results;
  std::string_view data(raw.data(), raw.size());

  size_t bodyStart = data.find("\r\n\r\n");
  size_t headerLen = 4;
  if (bodyStart == std::string_view::npos) {
    bodyStart = data.find("\n\n");
    headerLen = 2;
  }

  if (bodyStart == std::string_view::npos) {
    AttendanceEvent event;
    if (parse(raw, sourceIp, event) == ParseStatus::Success) {
      results.push_back(std::move(event));
    }
    return results;
  }

  std::string_view header = data.substr(0, bodyStart);
  std::string_view body = data.substr(bodyStart + headerLen);

  size_t lineStart = 0;
  bool isPositional = false;

  size_t firstNewline = body.find_first_of("\r\n");
  std::string_view firstLine = body.substr(
      0, firstNewline == std::string_view::npos ? std::string_view::npos
                                                : firstNewline);
  size_t firstTab = firstLine.find('\t');

  if (firstTab != std::string_view::npos && firstTab > 0) {
    bool allDigits = true;
    for (size_t i = 0; i < firstTab; ++i) {
      if (!std::isdigit(firstLine[i])) {
        allDigits = false;
        break;
      }
    }
    if (allDigits)
      isPositional = true;
  }

  if (isPositional) {
    while (lineStart < body.length()) {
      size_t lineEnd = body.find_first_of("\r\n", lineStart);
      std::string_view line = body.substr(
          lineStart, lineEnd == std::string_view::npos ? std::string_view::npos
                                                       : lineEnd - lineStart);

      if (!line.empty()) {
        bool allDigits = true;
        size_t tab = line.find('\t');
        if (tab != std::string_view::npos && tab > 0) {
          for (size_t i = 0; i < tab; ++i) {
            if (!std::isdigit(line[i])) {
              allDigits = false;
              break;
            }
          }
          if (allDigits) {
            std::string combinedPayload =
                std::string(header) + "\n\n" + std::string(line);
            AttendanceEvent event;
            if (parse(std::span<const char>(combinedPayload.data(),
                                            combinedPayload.size()),
                      sourceIp, event) == ParseStatus::Success) {
              results.push_back(std::move(event));
            }
          }
        }
      }

      if (lineEnd == std::string_view::npos)
        break;
      lineStart = lineEnd + 1;
      if (lineStart < body.length() && body[lineEnd] == '\r' &&
          body[lineStart] == '\n')
        lineStart++;
    }
  } else {
    AttendanceEvent event;
    if (parse(raw, sourceIp, event) == ParseStatus::Success) {
      results.push_back(std::move(event));
    }
  }

  return results;
}
