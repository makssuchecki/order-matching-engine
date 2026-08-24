#include <gtest/gtest.h>

#include "ome/flat_order_book.hpp"

using namespace ome;

using TestBook = FlatOrderBook<201, 10000>;

TEST(FlatOrderBook, PriceWithinRangeMapsToCorrectIndex){
    TestBook flat_book(0, 100);
    
    EXPECT_EQ(flat_book.price_to_index(0).value(), 0);
    EXPECT_EQ(flat_book.price_to_index(50).value(), 50);
    EXPECT_EQ(flat_book.price_to_index(100).value(), 100);
}

TEST(FlatOrderBook, PriceOutsideRangeReturnsNullopt){
    TestBook flat_book(0, 100);

    EXPECT_FALSE(flat_book.price_to_index(-1).has_value());
    EXPECT_FALSE(flat_book.price_to_index(101).has_value());
}

TEST(FlatOrderBook, AddingSingleBuyToEmptyBookHasNoTrades) {
    TestBook book(0, 200);
    Order order{.id = 1, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 1, .next = nullptr};

    auto trades = book.add_limit_order(order);

    EXPECT_TRUE(trades.empty());
    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(book.best_bid().value(), 100);
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(FlatOrderBook, NonCrossingOrdersRestInBookWithoutMatching) {
    TestBook book(0, 200);
    Order buy{.id = 1, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 1, .next = nullptr};
    Order sell{.id = 2, .price = 110, .quantity = 5, .side = Side::Sell, .timestamp = 2, .next = nullptr};

    auto trades_buy = book.add_limit_order(buy);
    auto trades_sell = book.add_limit_order(sell);

    EXPECT_TRUE(trades_buy.empty());
    EXPECT_TRUE(trades_sell.empty());
    EXPECT_EQ(book.best_bid().value(), 100);
    EXPECT_EQ(book.best_ask().value(), 110);
}

TEST(FlatOrderBook, FullyCrossingOrdersProduceOneTrade) {
    TestBook book(0, 200);
    Order sell{.id = 1, .price = 100, .quantity = 10, .side = Side::Sell, .timestamp = 1, .next = nullptr};
    Order buy{.id = 2, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 2, .next = nullptr};

    book.add_limit_order(sell);
    auto trades = book.add_limit_order(buy);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].buy_order_id, 2);
    EXPECT_EQ(trades[0].sell_order_id, 1);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 10);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(FlatOrderBook, CrossingOrderLeavesRest) {
    TestBook book(0, 200);
    Order sell{.id = 1, .price = 100, .quantity = 10, .side = Side::Sell, .timestamp = 1, .next = nullptr};
    Order buy{.id = 2, .price = 100, .quantity = 4, .side = Side::Buy, .timestamp = 2, .next = nullptr};

    book.add_limit_order(sell);
    auto trades = book.add_limit_order(buy);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].buy_order_id, 2);
    EXPECT_EQ(trades[0].sell_order_id, 1);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 4);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_TRUE(book.best_ask().has_value());
}

TEST(FlatOrderBook, TradesFirstInOrderLeavingQueue) {
    TestBook book(0, 200);
    Order sell1{.id = 1, .price = 100, .quantity = 5, .side = Side::Sell, .timestamp = 1, .next = nullptr};
    Order sell2{.id = 2, .price = 100, .quantity = 5, .side = Side::Sell, .timestamp = 2, .next = nullptr};
    Order buy{.id = 3, .price = 100, .quantity = 5, .side = Side::Buy, .timestamp = 3, .next = nullptr};

    book.add_limit_order(sell1);
    book.add_limit_order(sell2);
    auto trades = book.add_limit_order(buy);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].buy_order_id, 3);
    EXPECT_EQ(trades[0].sell_order_id, 1);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 5);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_TRUE(book.best_ask().has_value());
}