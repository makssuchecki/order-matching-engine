#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <optional>
#include <array>

#include "ome/order.hpp"
#include "ome/order_queue.hpp"
#include "ome/memory_pool.hpp"

namespace ome {

struct Trade {
    std::uint64_t buy_order_id;
    std::uint64_t sell_order_id;
    std::int64_t price;
    std::uint32_t quantity;
};

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

    std::vector<Trade> add_limit_order(Order order);
    std::optional<std::int64_t> best_bid() const;
    std::optional<std::int64_t> best_ask() const;

    std::optional<std::size_t> price_to_index(std::int64_t price) const {
        if (price < min_price_ || price > max_price_) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(price - min_price_);
    }
private:
    std::int64_t min_price_;
    std::int64_t max_price_;

    MemoryPool<Order, PoolCapacity> bid_pool_;
    MemoryPool<Order, PoolCapacity> ask_pool_;

    std::vector<OrderQueue<PoolCapacity>> bid_levels_;
    std::vector<OrderQueue<PoolCapacity>> ask_levels_;

};

}