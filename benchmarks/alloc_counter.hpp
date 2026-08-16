#pragma once

#include <cstdlib>
#include <atomic>

inline std::atomic<std::size_t> g_alloc_count{0};

void* operator new(std::size_t size) {
    g_alloc_count.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size);
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    std::free(ptr);
}