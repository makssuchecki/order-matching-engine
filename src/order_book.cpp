#include "ome/order_book.hpp"
#include <algorithm>

namespace ome {

std::optional<std::int64_t> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<std::int64_t> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::vector<Trade> OrderBook::add_limit_order(Order order) {
    std::vector<Trade> trades;

    if (order.side == Side::Buy){
        trades = match_buy(order);
        if (order.quantity > 0) {
            bids_[order.price].push_back(order);
        }
    } else{
        trades = match_sell(order);
        if (order.quantity > 0) {
            asks_[order.price].push_back(order);
        }
    }

    return trades;
}
std::vector<Trade> OrderBook::match_buy(Order& incoming) {
    std::vector<Trade> trades;    

    while (incoming.quantity > 0 && !asks_.empty()) {
        auto best_level = asks_.begin();
        std::int64_t best_price = best_level->first;

        if (best_price > incoming.price){
            break;
        }

        auto& level_orders = best_level->second;
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
            if (level_orders.empty()){
                asks_.erase(best_level);
            }
        }
    }

    return trades;
}

std::vector<Trade> OrderBook::match_sell(Order& incoming) {
    std::vector<Trade> trades;    

    while (incoming.quantity > 0 && !bids_.empty()){
        auto best_level = bids_.begin();
        std::int64_t best_price = best_level->first;

        if (best_price < incoming.price){
            break;
        }

        auto& level_orders = best_level->second;
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

        if (resting.quantity == 0){
            level_orders.pop_front();
            if (level_orders.empty()){
                bids_.erase(best_level);
            }
        }
    }
    return trades;
}

}

