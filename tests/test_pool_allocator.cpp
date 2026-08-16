#include <gtest/gtest.h>
#include <cstdint>
#include <deque>

#include "ome/pool_allocator.hpp"

using namespace ome;


TEST(PoolAllocator, DequeCompilesAndWorks){
    std::deque<int, PoolAllocator<int>> d;

    d.push_back(1);
    d.push_back(2);
    d.push_back(3);

    ASSERT_EQ(d.size(), 3);
    EXPECT_EQ(d.front(), 1);
    EXPECT_EQ(d.back(), 3);
}
TEST(PoolAllocator, PopFrontWorks){
    std::deque<int, PoolAllocator<int>> d;

    d.push_back(10);
    d.push_back(20);

    d.pop_front();

    ASSERT_EQ(d.size(), 1);
    EXPECT_EQ(d.front(), 20);
}
