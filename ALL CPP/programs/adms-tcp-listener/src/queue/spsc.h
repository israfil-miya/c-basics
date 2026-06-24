#pragma once

#include <atomic>
#include <cstddef>

/**
 * A lock-free Single-Producer Single-Consumer (SPSC) ring buffer queue.
 * Designed to hold fixed-size POD structs like AttendanceEvent.
 *
 * @tparam T The type of elements in the queue
 * @tparam Capacity The maximum number of elements + 1 (1 slot is left empty to distinguish full/empty)
 */
template<typename T, size_t Capacity>
class SPSCQueue {
public:
    SPSCQueue() : head_(0), tail_(0) {}

    // Delete copy and move constructors
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    /**
     * Pushes an item onto the queue.
     * @param item The item to copy into the queue.
     * @return true if pushed successfully, false if the queue is full.
     */
    bool push(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % Capacity;
        
        // Acquire barrier ensures we see the latest head_ update from consumer
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue is full
        }
        
        buffer_[current_tail] = item;
        
        // Release barrier ensures the data write is visible before the tail update
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    /**
     * Pops an item from the queue.
     * @param out_item The reference to store the popped item.
     * @return true if popped successfully, false if the queue is empty.
     */
    bool pop(T& out_item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        
        // Acquire barrier ensures we see the latest tail_ update from producer
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Queue is empty
        }
        
        out_item = buffer_[current_head];
        
        // Release barrier ensures the data read is complete before the head update
        head_.store((current_head + 1) % Capacity, std::memory_order_release);
        return true;
    }

    /**
     * Check if queue is empty (approximate, for polling).
     */
    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

private:
    T buffer_[Capacity];
    
    // Align atomic variables to typical cache line sizes (64 bytes) to prevent false sharing
    // between the producer and consumer threads.
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};
