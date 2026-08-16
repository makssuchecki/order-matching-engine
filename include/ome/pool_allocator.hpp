#pragma once

#include <array>
#include <cstddef>
#include "ome/memory_pool.hpp"

namespace ome {

template <typename T>
class PoolAllocator {
public:
    using value_type = T;

    PoolAllocator() noexcept = default;

    template <typename U>
    PoolAllocator(const PoolAllocator<U>&) noexcept {}

    T* allocate(std::size_t n){
        if (n != 1) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        return pool().allocate();
    }
    void deallocate(T* ptr, std::size_t n) noexcept {
        if (n != 1){
            ::operator delete(ptr);
            return;
        }
        pool().deallocate(ptr);
    }
private:
    static MemoryPool<T, 10000>& pool() {
        static MemoryPool<T, 10000> instance;
        return instance;
    }
};

template <typename T, typename U>
bool operator==(const PoolAllocator<T>&, const PoolAllocator<U>&) noexcept { return true; }

template <typename T, typename U>
bool operator!=(const PoolAllocator<T>&, const PoolAllocator<U>&) noexcept { return false; }

}