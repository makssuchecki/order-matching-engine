# Order Matching Engine

A low-latency limit order book (mini-exchange) written in C++20, built as a
learning project focused on performance engineering: lock-free concurrency,
custom memory allocation, and cache-friendly data structures — each backed
by benchmarks, not assumptions.

## What it does

- Maintains a limit order book for a single instrument
- Matches buy/sell orders on price-time priority
- Supports partial fills
- Processes an incoming order stream via a lock-free SPSC queue between a
  producer thread and the matching engine thread

## Why

Most portfolio projects show a finished result. This one is built to show
the *process*: a naive baseline, a measured bottleneck, an attempted fix,
and — twice — a first attempt that made things worse before the second
attempt made things better. Every performance claim below is backed by a
benchmark run on the same machine, with the raw numbers kept.

## Architecture

order-matching-engine/
├── include/ome/
│ ├── order.hpp # Order, Side (POD, trivially copyable)
│ ├── trade.hpp # Trade result type
│ ├── order_book.hpp # std::map-based book (baseline)
│ ├── flat_order_book.hpp # array-indexed book (optimized)
│ ├── order_queue.hpp # intrusive FIFO queue, pool-backed
│ ├── memory_pool.hpp # fixed-capacity free-list allocator
│ ├── pool_allocator.hpp # STL-compatible wrapper (see Stage 2 notes)
│ └── spsc_queue.hpp # lock-free single-producer/single-consumer queue
├── src/
├── tests/ # GoogleTest, unit + multi-threaded
├── benchmarks/ # Google Benchmark
└── docs/design.md # full stage-by-stage writeup with all numbers

## Build

Requires CMake 3.20+, a C++20 compiler (tested with GCC 13). Dependencies
(GoogleTest, Google Benchmark) are fetched automatically via CPM.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/ome_tests
./build/ome_benchmarks --benchmark_min_time=1s
```

## Performance summary

All numbers below: 8-core 3.0 GHz CPU, `-O3 -march=native`, GoogleTest/Benchmark
via CPM. Full methodology and every intermediate measurement — including the
regressions — is in [`docs/design.md`](docs/design.md).

| Stage | What changed | Result |
|---|---|---|
| 1. Baseline | `std::map` + `std::deque` order book | 18 956 ns / 1000 orders, 200 allocations |
| 2. Memory pool | Custom free-list allocator + intrusive queue | 200 → 50 allocations. First attempt (per-price-level pool) caused a **400x regression**; fixed by sharing one pool per book side. |
| 3. Lock-free queue | SPSC ring buffer, `acquire`/`release` semantics | Verified race-free under ThreadSanitizer; a deliberately broken (`relaxed`-only) version was confirmed to trigger a real data race, validating the test actually proves something |
| 4. Flat order book | Array-indexed price levels replacing `std::map` | First attempt (linear best-price scan) was **56x slower** than baseline. Tracking the best-price boundary incrementally fixed it: final result is **1.7–1.85x faster than the `std::map` baseline**. |

## Design decisions worth reading about

- **Integer prices, not floating point** — prices are stored as `int64_t`
  in the smallest tick unit, avoiding rounding error accumulation.
- **Trivially copyable `Order`** — verified via `static_assert`, enables
  safe use in the lock-free queue and the memory pool without custom
  copy/move logic.

## Testing

- Unit tests for every component (order book matching, memory pool,
  SPSC queue) including price-time priority and partial-fill edge cases
- Multi-threaded producer/consumer test for the SPSC queue (100,000
  elements, verifies zero loss/duplication)

## A note on AI-assisted development
Parts of this project were built with Claude as a technical mentor -
explaining concepts. All code in this repository was typed, debugged, and
understood by me; Claude did not write files directly into this repo.