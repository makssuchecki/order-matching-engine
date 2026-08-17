#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <vector>

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

class OrderBook {
public:
    OrderBook() = default;

    std::vector<Trade> add_limit_order(Order order);

    std::optional<std::int64_t> best_bid() const;
    std::optional<std::int64_t> best_ask() const;

private:
    static constexpr std::size_t kPoolCapacity = 100000;

    MemoryPool<Order, kPoolCapacity> bid_pool_;
    MemoryPool<Order, kPoolCapacity> ask_pool_;

    std::map<std::int64_t, OrderQueue<kPoolCapacity>, std::greater<>> bids_;
    std::map<std::int64_t, OrderQueue<kPoolCapacity>> asks_;

    std::vector<Trade> match_buy(Order& incoming);
    std::vector<Trade> match_sell(Order& incoming);
};

}