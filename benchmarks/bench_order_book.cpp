#include <benchmark/benchmark.h>

#include "ome/order_book.hpp"
#include "ome/order.hpp"

static void BM_AddNonCrossingOrders(benchmark::State& state) {
    std::vector<ome::Order> orders;
    orders.reserve(1000);

    for (int i=0; i<1000; i++){
        orders.push_back(ome::Order{
            .id = static_cast<std::uint64_t>(i),
            .price = (i% 2 == 0) ? (9000 + (i % 50)) : (11000 + (i % 50)),
            .quantity = 10,
            .side = (i % 2 == 0) ? ome::Side::Buy : ome::Side::Sell,
            .timestamp = static_cast<std::uint64_t>(i)
        });
    }

    for (auto _ : state) {
        ome::OrderBook book;
        for (const auto& order : orders){
            book.add_limit_order(order);
        }
    }
}
BENCHMARK(BM_AddNonCrossingOrders);

BENCHMARK_MAIN();