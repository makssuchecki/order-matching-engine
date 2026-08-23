#include <gtest/gtest.h>

#include "ome/spsc_queue.hpp"

using namespace ome;

TEST(SpscQueue, PopReturnsElementsInFifoOrder) {
    SpscQueue<int, 8> spsc;

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

TEST(SpscQueue, FullPushReturnsFalse) {
    SpscQueue<int, 4> spsc;

    EXPECT_TRUE(spsc.push(5));
    EXPECT_TRUE(spsc.push(10));
    EXPECT_TRUE(spsc.push(20));
    EXPECT_TRUE(spsc.push(40));

    EXPECT_FALSE(spsc.push(999)); // bufor pełny, piąty push się nie uda
}

TEST(SpscQueue, EmptyPopReturnsFalse) {
    SpscQueue<int, 4> spsc;

    int value;
    EXPECT_FALSE(spsc.pop(value));
}

TEST(SpscQueue, PopAfterFullFreesSlotForNextPush) {
    SpscQueue<int, 4> spsc;

    spsc.push(1);
    spsc.push(2);
    spsc.push(3);
    spsc.push(4); // pełne

    int value;
    ASSERT_TRUE(spsc.pop(value));
    EXPECT_EQ(value, 1);

    EXPECT_TRUE(spsc.push(5)); // teraz jest miejsce
}