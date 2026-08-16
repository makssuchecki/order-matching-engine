#include <benchmark/benchmark.h>
#include <iostream>

#include "ome/order_book.hpp"
#include "ome/order.hpp"
#include "alloc_counter.hpp"

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
    
    std::size_t before = g_alloc_count.load();
    {
        ome::OrderBook book;
        for (const auto& order : orders) {
            book.add_limit_order(order);
        }
    }
    std::size_t after = g_alloc_count.load();
    std::cerr << "Allocations for 1000 orders: " << (after - before) << "\n";

    for (auto _ : state) {
        ome::OrderBook book;
        for (const auto& order : orders){
            book.add_limit_order(order);
        }
    }
}

static void BM_AddCrossingOrders(benchmark::State& state){
    std::vector<ome::Order> resting_sells;
    resting_sells.reserve(500);
    for (int i=0; i<500; i++){
        resting_sells.push_back(ome::Order{
            .id = static_cast<std::uint64_t>(i),
            .price = 10000 + i,
            .quantity = 10,
            .side = ome::Side::Sell,
            .timestamp = static_cast<std::uint64_t>(i)
        });
    }
    std::vector<ome::Order> crossing_buys;
    crossing_buys.reserve(500);
    for (int i=0; i<500; i++){
        crossing_buys.push_back(ome::Order{
            .id = static_cast<std::uint64_t>(i),
            .price = 10000 + i,
            .quantity = 10,
            .side = ome::Side::Buy,
            .timestamp = static_cast<std::uint64_t>(i)
        });
    }

    for (auto _ : state){
        state.PauseTiming();
        ome::OrderBook book;
        for (const auto& sell : resting_sells) {
            book.add_limit_order(sell);
        }
        state.ResumeTiming();
        for (const auto& buy : crossing_buys){
            book.add_limit_order(buy);
        }
    }
}

BENCHMARK(BM_AddNonCrossingOrders);
BENCHMARK(BM_AddCrossingOrders);

BENCHMARK_MAIN();