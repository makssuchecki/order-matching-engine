#include <gtest/gtest.h>

#include "ome/order_book.hpp"

using namespace ome;

TEST(OrderBook, AddingSingleBuyToEmptyBookHasNoTrades) { 
    OrderBook book;
    Order order{.id = 1, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 1};

    auto trades = book.add_limit_order(order);

    EXPECT_TRUE(trades.empty());
    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(book.best_bid().value(), 100);
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(OrderBook, NonCrossingOrdersRestInBookWithoutMatching) { 
    OrderBook book;
    Order buy{.id = 1, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 1};
    Order sell{.id = 2, .price = 110, .quantity = 5, .side = Side::Sell, .timestamp = 2};

    auto trades_buy = book.add_limit_order(buy);
    auto trades_sell = book.add_limit_order(sell);

    EXPECT_TRUE(trades_buy.empty());
    EXPECT_TRUE(trades_sell.empty());
    EXPECT_EQ(book.best_bid().value(), 100);
    EXPECT_EQ(book.best_ask().value(), 110);
}
