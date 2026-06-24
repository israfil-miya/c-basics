#pragma once

#include "../queue/spsc.h"
#include "poster.h"
#include <atomic>
#include <chrono>
#include <thread>

class Worker {
public:
  using EventQueue = SPSCQueue<AttendanceEvent, 1024>;

  Worker(EventQueue &queue, const ApiConfig &apiConfig)
      : queue_(queue), apiClient_(apiConfig), running_(false) {}

  ~Worker() { stop(); }

  void start() {
    if (running_)
      return;
    running_ = true;
    thread_ = std::thread(&Worker::run, this);
  }

  void stop() {
    if (!running_)
      return;
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

private:
  void run() {
    AttendanceEvent event;
    while (running_) {
      // Drain queue as fast as possible
      bool processed_any = false;
      while (queue_.pop(event)) {
        processed_any = true;
        // Blocking HTTP POST - Since we reuse the libcurl handle,
        // this is extremely fast and keeps the connection alive.
        apiClient_.sendEvent(event);
      }

      // If queue was empty, sleep lightly to prevent 100% CPU spinning
      if (!processed_any) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    }

    // Final drain before exiting to ensure no events are lost
    while (queue_.pop(event)) {
      apiClient_.sendEvent(event);
    }
  }

  EventQueue &queue_;
  CloudApiClient apiClient_;
  std::atomic<bool> running_;
  std::thread thread_;
};
