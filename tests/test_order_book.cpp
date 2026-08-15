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

TEST(OrderBook, FullyCrossingOrdersProduceOneTrade) {
    OrderBook book;
    Order sell{.id = 1, .price = 100, .quantity = 10, .side = Side::Sell, .timestamp = 1};
    Order buy{.id = 2, .price = 100, .quantity = 10, .side = Side::Buy, .timestamp = 2};

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

TEST(OrderBook, CrossingOrderLeavesRest){
    OrderBook book;
    Order sell{.id = 1, .price = 100, .quantity = 10, .side = Side::Sell, .timestamp = 1};
    Order buy{.id = 2, .price = 100, .quantity = 4, .side = Side::Buy, .timestamp = 2};

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

TEST(OrderBook, TradesFirstInOrderLeavingQueue) {
    OrderBook book;
    Order sell1{.id = 1, .price = 100, .quantity = 5, .side = Side::Sell, .timestamp = 1};
    Order sell2{.id = 2, .price = 100, .quantity = 5, .side = Side::Sell, .timestamp = 2};
    Order buy{.id = 3, .price = 100, .quantity = 5, .side = Side::Buy, .timestamp = 3};

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
