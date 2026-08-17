## Stage 1: Naive implementation (baseline)

### Architecture
- `std::map<price, std::deque<Order>, std::greater<>>` for bids
- `std::map<price, std::deque<Order>>` for asks
- Price-time priority achieved via natural `std::deque` ordering
  (push_back/front) — no explicit timestamp-based sorting

### Design decisions
- Price stored as `std::int64_t` in the smallest unit (not `double`) —
  avoids floating-point rounding errors
- `Order` is a POD type, `sizeof(Order) == 32` bytes (verified via
  `static_assert(std::is_trivially_copyable_v<Order>)`)

### Benchmark results
| Scenario | Time / batch | Time / order | Iterations |
|---|---|---|---|
| Insert 1000 non-crossing orders | 25 676 ns | ~25.7 ns | 27 650 |
| Insert 500 crossing buy orders against 500 resting sells | 50 923 ns | ~101.8 ns/pair | 14 418 |
| Allocations per 1000 orders (non-crossing insert) | 200 (~100 std::map nodes, ~100 std::deque blocks) | — | — |

## Stage 2: Memory pool / custom allocator

### What was built
- `MemoryPool<T, BlockCount>` — fixed-capacity free-list allocator,
  O(1) allocate/deallocate, no syscalls after construction. Verified
  via unit tests (distinct pointers on repeated allocate, slot reuse
  after deallocate).
- `PoolAllocator<T>` — STL-compatible allocator wrapping `MemoryPool`,
  verified to work as a drop-in allocator for `std::deque` (push_back,
  pop_front, iteration).

### Benchmark results (std::deque<Order, PoolAllocator<Order>>)
| Scenario | Time / batch | Allocations / 1000 orders |
|---|---|---|
| Insert 1000 non-crossing orders | 24 548 ns | 200 (unchanged from baseline) |

### Finding: PoolAllocator ineffective with std::deque
`PoolAllocator::allocate(n)` only serves the fast path when `n == 1`,
falling back to `::operator new` otherwise. libstdc++'s `std::deque`
allocates memory in fixed-size chunks holding multiple elements per
call (n > 1 in practice), so the pool's fast path was never exercised
— allocation count and timing remained statistically identical to the
`std::deque<Order>` baseline.


### Finding: per-price-level MemoryPool causes catastrophic overhead

Sizing each price level's OrderQueue<10000> (~390 KB per level, due to
MemoryPool<Order, 10000>'s std::array<Slot, 10000>) caused benchmark
time to regress from ~25 μs to ~11 ms per 1000-order batch — a >400x
slowdown. Root cause: constructing a new price level triggers a
390 KB allocation plus a 10,000-iteration free-list initialization
loop, executed once per unique price level (100 levels in the
benchmark = ~39 MB touched per OrderBook construction).

This confirms the earlier design assumption (one pool per level) was
flawed at this scale — the fix is a single, shared MemoryPool across
all price levels, sized once for the whole order book's expected
capacity, not one pool per level.


## Stage 2: Memory pool / custom allocator (continued)

### Iteration 2: shared MemoryPool + intrusive OrderQueue
Replaced `std::deque<Order, PoolAllocator<Order>>` with a custom
intrusive singly-linked queue (`OrderQueue`), backed by a single
`MemoryPool<Order, 100000>` shared across all price levels on each
side of the book (one pool for bids, one for asks), rather than one
pool per price level.

First attempt (one MemoryPool per price level) caused a >400x
regression (~11 ms per 1000-order batch) due to each new price level
paying the full cost of constructing a 390 KB pool and initializing
a 10,000-slot free-list. Moving to a single shared pool per side,
constructed once per OrderBook, resolved this.

### Benchmark results
| Scenario | Baseline | This stage | Change |
|---|---|---|---|
| Insert 1000 non-crossing orders | 25 676 ns / 200 allocs | 30 086 ns / 50 allocs | time +17%, allocations -75% |
| 500 crossing buy vs 500 resting sells | 50 923 ns | 35 265 ns | time -31% |

### Notes
- Non-crossing insert is slightly slower than the std::deque baseline,
  likely due to OrderQueue's manual head/tail pointer bookkeeping and
  full Order copy into the pool slot, versus std::deque's internal
  block management. The crossing scenario, which exercises the actual
  matching hot path more heavily, shows a clear net win.
- OrderBook construction cost (~8 MB across two pools, 200,000 slots
  total) is now paid once per OrderBook lifetime, not per operation —
  appropriate for a long-lived book, but must be excluded from
  per-operation benchmarks (measured separately via PauseTiming).