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