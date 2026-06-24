#include <cerrno>
#include <cstring>
#include <curl/curl.h>
#include <string>
#include <array>
#include <memory>

#include <asio.hpp>

#include "http/worker.h"
#include "proto/parser.h"
#include "log/logger.h"
#include "config.h"

// Helper to determine if an HTTP request is complete
bool isHttpRequestComplete(const std::string &data) {
  if (data.empty())
    return false;

  auto startsWith = [](const std::string &s, const char *prefix) {
    return s.compare(0, strlen(prefix), prefix) == 0;
  };

  if (!startsWith(data, "GET ") && !startsWith(data, "POST ") &&
      !startsWith(data, "PUT ") && !startsWith(data, "DELETE ")) {
    return true;
  }

  size_t headerEnd = data.find("\r\n\r\n");
  if (headerEnd == std::string::npos)
    return false;

  size_t clPos = data.find("Content-Length:");
  if (clPos != std::string::npos && clPos < headerEnd) {
    size_t valueStart = clPos + 15;
    size_t valueEnd = data.find("\r\n", valueStart);
    if (valueEnd != std::string::npos && valueEnd < headerEnd) {
      std::string clStr = data.substr(valueStart, valueEnd - valueStart);
      long contentLength = strtol(clStr.c_str(), nullptr, 10);
      size_t actualBodyLength = data.length() - (headerEnd + 4);
      return actualBodyLength >= static_cast<size_t>(contentLength);
    }
  }
  return true;
}

// Session handles connection lifetime, reading, parsing and replying
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(asio::ip::tcp::socket socket, Worker::EventQueue &queue)
      : socket_(std::move(socket)), queue_(queue) {
    std::error_code ec;
    auto endpoint = socket_.remote_endpoint(ec);
    if (!ec) {
      remoteIp_ = endpoint.address().to_string();
    } else {
      remoteIp_ = "unknown";
    }
  }

  void start() {
    read();
  }

private:
  void read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        asio::buffer(buffer_),
        [this, self](std::error_code ec, std::size_t length) {
          if (!ec) {
            data_.append(buffer_.data(), length);
            if (isHttpRequestComplete(data_)) {
              handleRequest();
            } else {
              read();
            }
          }
        });
  }

  void handleRequest() {
    if (data_.compare(0, 11, "GET /health") == 0) {
      std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
      write(response);
      data_.clear();
      return;
    }

    auto events = AdmsParser::parseMultiple(data_, remoteIp_.c_str());
    for (const auto &ev : events) {
      Logger::getInstance().info("Attendance event parsed (User: " + std::string(ev.userId) + ")");
      Logger::getInstance().logAttendance(ev);
      if (!queue_.push(ev)) {
        Logger::getInstance().error("SPSC queue full, dropping event!");
      }
    }

    bool isHttp = (data_.compare(0, 4, "GET ") == 0 ||
                   data_.compare(0, 5, "POST ") == 0);
    std::string response;
    if (isHttp) {
      response =
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
          "2\r\nConnection: keep-alive\r\n\r\nOK";
    } else {
      response = "OK\n";
    }

    write(response);
    data_.clear();
  }

  void write(const std::string &response) {
    auto self(shared_from_this());
    auto writeBuffer = std::make_shared<std::string>(response);
    asio::async_write(
        socket_, asio::buffer(*writeBuffer),
        [this, self, writeBuffer](std::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            read(); // Wait for the next request on the same connection
          }
        });
  }

  asio::ip::tcp::socket socket_;
  Worker::EventQueue &queue_;
  std::array<char, 4096> buffer_;
  std::string data_;
  std::string remoteIp_;
};

// Server class managing port binding and connection acceptance
class Server {
public:
  Server(asio::io_context &io_context, short port, Worker::EventQueue &queue)
      : acceptor_(io_context), queue_(queue) {
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    accept();
  }

private:
  void accept() {
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket), queue_)->start();
          } else {
            Logger::getInstance().warn("Failed to accept connection: " + ec.message());
          }
          accept();
        });
  }

  asio::ip::tcp::acceptor acceptor_;
  Worker::EventQueue &queue_;
};

int main() {
  curl_global_init(CURL_GLOBAL_ALL);

  AppConfig appConfig = AppConfig::load();
  Logger::initialize(appConfig);
  Logger::getInstance().info("ZKTeco ADMS Listener starting...");

  ApiConfig apiConfig;
  apiConfig.cloudApiUrl = appConfig.cloudApiUrl;
  apiConfig.cloudApiAuthHeader = appConfig.cloudApiAuthHeader;
  apiConfig.cloudApiAuthValue = appConfig.cloudApiAuthValue;
  apiConfig.cloudApiTimeoutMs = appConfig.cloudApiTimeoutMs;

  Worker::EventQueue queue;
  Worker worker(queue, apiConfig);
  worker.start();

  try {
    asio::io_context io_context;
    Server server(io_context, appConfig.admsPort, queue);
    
    Logger::getInstance().info("Service ready. Waiting for device connections on port " + std::to_string(appConfig.admsPort));
    io_context.run();
  } catch (const std::exception &e) {
    Logger::getInstance().error("Server exception: " + std::string(e.what()));
  }

  worker.stop();
  curl_global_cleanup();
  return 0;
}
