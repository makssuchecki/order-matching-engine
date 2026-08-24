#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <optional>

#include "ome/order.hpp"
#include "ome/order_queue.hpp"
#include "ome/memory_pool.hpp"
#include "ome/trade.hpp"

namespace ome {

template <std::size_t PriceLevels, std::size_t PoolCapacity>
class FlatOrderBook {
public:
    FlatOrderBook(std::int64_t min_price, std::int64_t max_price)
        : min_price_(min_price), max_price_(max_price) {
        bid_levels_.reserve(PriceLevels);
        ask_levels_.reserve(PriceLevels);
        for (std::size_t i = 0; i < PriceLevels; i++) {
            bid_levels_.emplace_back(bid_pool_);
            ask_levels_.emplace_back(ask_pool_);
        }
    }

    std::optional<std::size_t> price_to_index(std::int64_t price) const {
        if (price < min_price_ || price > max_price_) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(price - min_price_);
    }

    std::optional<std::int64_t> best_bid() const {
        if (!best_bid_index_.has_value()) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(best_bid_index_.value()) + min_price_;
    }

    std::optional<std::int64_t> best_ask() const {
        if (!best_ask_index_.has_value()) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(best_ask_index_.value()) + min_price_;
    }

    std::vector<Trade> add_limit_order(Order order) {
        std::vector<Trade> trades;

        if (order.side == Side::Buy) {
            trades = match_buy(order);
            if (order.quantity > 0) {
                auto index = price_to_index(order.price).value();
                bid_levels_[index].push_back(order);

                if (!best_bid_index_.has_value() || index > best_bid_index_.value()) {
                    best_bid_index_ = index;
                }
            }
        } else {
            trades = match_sell(order);
            if (order.quantity > 0) {
                auto index = price_to_index(order.price).value();
                ask_levels_[index].push_back(order);

                if (!best_ask_index_.has_value() || index < best_ask_index_.value()) {
                    best_ask_index_ = index;
                }
            }
        }

        return trades;
    }

    std::vector<Trade> match_buy(Order& incoming) {
        std::vector<Trade> trades;

        while (incoming.quantity > 0) {
            auto ask_price = best_ask();
            if (!ask_price.has_value() || ask_price.value() > incoming.price) {
                break;
            }
            std::size_t index = best_ask_index_.value();
            auto& level_orders = ask_levels_[index];
            Order& resting = level_orders.front();

            std::uint32_t traded_quantity = std::min(incoming.quantity, resting.quantity);

            trades.push_back(Trade{
                .buy_order_id = incoming.id,
                .sell_order_id = resting.id,
                .price = resting.price,
                .quantity = traded_quantity
            });

            incoming.quantity -= traded_quantity;
            resting.quantity -= traded_quantity;

            if (resting.quantity == 0) {
                level_orders.pop_front();

                if (level_orders.empty()) {
                    // Ten poziom był granicą (best_ask_index_) i się opróżnił.
                    // Znajdź następny zajęty poziom w górę.
                    best_ask_index_ = std::nullopt;
                    for (std::size_t i = index + 1; i < ask_levels_.size(); i++) {
                        if (!ask_levels_[i].empty()) {
                            best_ask_index_ = i;
                            break;
                        }
                    }
                }
            }
        }
        return trades;
    }

    std::vector<Trade> match_sell(Order& incoming) {
        std::vector<Trade> trades;

        while (incoming.quantity > 0) {
            auto bid_price = best_bid();
            if (!bid_price.has_value() || bid_price.value() < incoming.price) {
                break;
            }
            std::size_t index = best_bid_index_.value();
            auto& level_orders = bid_levels_[index];
            Order& resting = level_orders.front();

            std::uint32_t traded_quantity = std::min(incoming.quantity, resting.quantity);

            trades.push_back(Trade{
                .buy_order_id = resting.id,
                .sell_order_id = incoming.id,
                .price = resting.price,
                .quantity = traded_quantity
            });

            incoming.quantity -= traded_quantity;
            resting.quantity -= traded_quantity;

            if (resting.quantity == 0) {
                level_orders.pop_front();

                if (level_orders.empty()) {
                    // Ten poziom był granicą (best_bid_index_) i się opróżnił.
                    // Znajdź następny zajęty poziom w dół.
                    best_bid_index_ = std::nullopt;
                    for (std::size_t i = index; i-- > 0; ) {
                        if (!bid_levels_[i].empty()) {
                            best_bid_index_ = i;
                            break;
                        }
                    }
                }
            }
        }
        return trades;
    }

private:
    std::int64_t min_price_;
    std::int64_t max_price_;

    MemoryPool<Order, PoolCapacity> bid_pool_;
    MemoryPool<Order, PoolCapacity> ask_pool_;

    std::vector<OrderQueue<PoolCapacity>> bid_levels_;
    std::vector<OrderQueue<PoolCapacity>> ask_levels_;

    std::optional<std::size_t> best_bid_index_;
    std::optional<std::size_t> best_ask_index_;
};

} // namespace ome