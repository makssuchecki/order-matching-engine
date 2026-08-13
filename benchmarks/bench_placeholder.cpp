#include <benchmark/benchmark.h>

static void bm_placeholder(benchmark::State& state){
    for (auto _ : state){
        int x = 0;
        benchmark::DoNotOptimize(x += 1);
    }
}

BENCHMARK(bm_placeholder);

BENCHMARK_MAIN();