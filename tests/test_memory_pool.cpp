#include <gtest/gtest.h>
#include <vector>

#include "ome/memory_pool.hpp"

using namespace ome;

TEST(MemoryPool, AllocateNTimesReturnsDistinctNonNullPointers){
    MemoryPool<int, 1000> mem_pool;
    std::vector<int*> pointers;

    for (int i=0; i<100; i++){
        int* ptr = mem_pool.allocate();
        ASSERT_NE(ptr, nullptr);
        pointers.push_back(ptr);
    }

    for (std::size_t i = 0; i<pointers.size(); i++){
        for (std::size_t j = i + 1; j<pointers.size(); j++){
            EXPECT_NE(pointers[i], pointers[j]);
        }
    }
}

TEST(MemoryPool, DeallocateThenAllocateReturnsSameSlot) {
    MemoryPool<int, 1000> mem_pool;
    
    int* first = mem_pool.allocate();
    mem_pool.deallocate(first);
    int* second = mem_pool.allocate();

    EXPECT_EQ(first, second);
}