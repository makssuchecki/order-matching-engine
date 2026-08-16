#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "ome/order.hpp"
#include "ome/pool_allocator.hpp"

namespace ome {
struct Trade {
    std::uint64_t buy_order_id;
    std::uint64_t sell_order_id;
    std::int64_t price;
    std::uint32_t quantity;
};

class OrderBook {
public:
    std::vector<Trade> add_limit_order(Order order);

    std::optional<std::int64_t> best_bid() const;
    std::optional<std::int64_t> best_ask() const;

private:
    std::map<std::int64_t, std::deque<Order, PoolAllocator<Order>>, std::greater<>> bids_;

    std::map<std::int64_t, std::deque<Order, PoolAllocator<Order>>> asks_;

    std::vector<Trade> match_buy(Order& incoming);
    std::vector<Trade> match_sell(Order& incoming);
};

}