#include <gtest/gtest.h>

#include "ome/order_queue.hpp"
#include "ome/order.hpp"

using namespace ome;

TEST(OrderQueue, PushFrontMaintainsFifoOrder ){
    OrderQueue<1000> q;
    Order first{.id = 1, .price=100, .quantity = 10, .side = Side::Buy, .timestamp = 1, .next = nullptr};
    Order second{.id = 2, .price=200, .quantity = 10, .side = Side::Buy, .timestamp = 2, .next = nullptr};

    q.push_back(first);
    q.push_back(second);
    
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.front().id, 1);
    q.pop_front();
    EXPECT_EQ(q.front().id, 2);
}

TEST(OrderQueue, EmptyAfterPoppingAllElements){
    OrderQueue<1000> q;

    Order only{.id = 1, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 1, .next = nullptr};
    q.push_back(only);
    
    EXPECT_FALSE(q.empty());
    q.pop_front();
    EXPECT_TRUE(q.empty());
}