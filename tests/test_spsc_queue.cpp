#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "ome/spsc_queue.hpp"

using namespace ome;

TEST(SpscQueue, SingleThread){
    ome::SpscQueue<int, 8> spsc;

    spsc.push(5);
    spsc.push(10);
    spsc.push(20);

    int first, second, third;
    ASSERT_TRUE(spsc.pop(first));
    ASSERT_TRUE(spsc.pop(second));
    ASSERT_TRUE(spsc.pop(third));

    EXPECT_EQ(first, 5);
    EXPECT_EQ(second, 10);
    EXPECT_EQ(third, 20);
}

TEST(SpscQueue, FullPushReturnsFalse){
    ome::SpscQueue<int, 4> spsc;

    EXPECT_TRUE(spsc.push(5));
    EXPECT_TRUE(spsc.push(10));
    EXPECT_TRUE(spsc.push(20));
    EXPECT_TRUE(spsc.push(40));

    EXPECT_FALSE(spsc.push(99));
}

TEST(SpscQueue, EmptyPopReturnsFalse){
    SpscQueue<int, 4> spsc;
    
    int value;
    EXPECT_FALSE(spsc.pop(value));
}
TEST(SpscQueue, PopAfterFullFreesSlotsForNextPush){
    ome::SpscQueue<int, 4> spsc;
    spsc.push(1);
    spsc.push(2);
    spsc.push(3);
    spsc.push(4);

    int value;
    ASSERT_TRUE(spsc.pop(value));
    EXPECT_EQ(value, 1);

    EXPECT_TRUE(spsc.push(5));
}

TEST(SpscQueue, ConcurrentProducerConsumerPreserverOrder){
    constexpr int kCount = 100000;
    SpscQueue<int, 1024> spsc;
    std::vector<int> consumed;
    consumed.reserve(kCount);

    std::thread producer([&spsc]() {
        for (int i = 0; i < kCount; i++){
            while (!spsc.push(i)){
                
            }
        }
    });
    std::thread consumer([&spsc, &consumed]() {
        int value;
        for (int i = 0; i < kCount; i++){
            while (!spsc.pop(value)){

            }
            consumed.push_back(value);
        }
    });
    producer.join();
    consumer.join();
    ASSERT_EQ(consumed.size(), kCount);
    for (int i = 0; i < kCount; i++){
        EXPECT_EQ(consumed[i], i);
    }
}
