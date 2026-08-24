#include <gtest/gtest.h>

#include "ome/flat_order_book.hpp"

using namespace ome;

TEST(FlatOrderBook, PriceWithinRangeMapsToCorrectIndex){
    FlatOrderBook<101, 10000> flat_book(0, 100);
    
    EXPECT_EQ(flat_book.price_to_index(0).value(), 0);
    EXPECT_EQ(flat_book.price_to_index(50).value(), 50);
    EXPECT_EQ(flat_book.price_to_index(100).value(), 100);
};

TEST(FlatOrderBook, PriceOutsideRangeReturnsNullopt){
    FlatOrderBook<101, 10000> flat_book(0, 100);

    EXPECT_FALSE(flat_book.price_to_index(-1).has_value());
    EXPECT_FALSE(flat_book.price_to_index(101).has_value());
}