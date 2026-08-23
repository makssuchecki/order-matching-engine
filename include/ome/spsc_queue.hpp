#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace ome {
    
template <typename T, std::size_t Capacity>
class SpscQueue {
    static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity must be a power of 2 (enables & mask_ instead of %)");
public:
    SpscQueue() : head_(0), tail_(0) {}

    bool push (const T& item){
        auto current_tail = tail_.load(std::memory_order_relaxed);
        auto current_head = head_.load(std::memory_order_acquire);

        if (current_tail - current_head == Capacity){
            return false;
        }
        buffer_[current_tail & mask_] = item;
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& out){
        auto current_head = head_.load(std::memory_order_relaxed);
        auto current_tail = tail_.load(std::memory_order_acquire);
        if (current_head == current_tail){
            return false;
        } 
        out = buffer_[current_head & mask_];
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }
    bool empty() const {
        return head_.load(std::memory_order_acquire) == 
                tail_.load(std::memory_order_acquire);
    }
private:
    static constexpr std::size_t mask_ = Capacity - 1;
    
    std::array<T, Capacity> buffer_;
    std::atomic<std::size_t> head_;
    std::atomic<std::size_t> tail_;

};   
}