# 🧠 memory-allocator

A slab allocator built from scratch in C++ — the same architecture behind the Linux kernel's SLUB allocator and Google's tcmalloc. Not a toy, not a tutorial follow-along. Built layer by layer with a benchmark to back it up.

---

## Why does this exist?

`malloc` is general purpose. It handles every size, every pattern, every edge case — and that generality has a cost. It stores metadata next to every allocation, coalesces free regions to fight fragmentation, and uses a global lock that becomes a bottleneck under multiple threads.

If you *know* your allocation patterns — and in systems software you usually do — you can do better. This project is proof of that.

---

## 🏗️ Architecture

Three layers, each one sitting on top of the last:

```text
your code
    ↓
CacheAllocator     — thread-local block stash per size class, minimizes locking
    ↓
SlabAllocator      — routes requests to the right pool based on size
    ↓
Pool               — fixed-size block pool backed by a free-list
    ↓
OS memory          — raw pages via mmap (Linux) or malloc (Windows)
```

### Layer 1 — Pool 🏊

The foundation. Grabs a big chunk of raw memory from the OS upfront and carves it into equal-sized blocks. Tracks free blocks using a **free-list** — a linked list where each free block's first bytes store the address of the next free block. No separate metadata array, no searching.

- `allocate()` — pop the head of the free list. O(1).
- `deallocate()` — push back to the head. O(1).

### Layer 2 — Slab Allocator 🗂️

Eight pools, one per size class:

```text
8B | 16B | 32B | 64B | 128B | 256B | 512B | 1024B
```

When you call `allocate(n)`, it rounds up to the nearest size class and delegates to that pool. Requests above 1024 bytes return `nullptr` — out of scope for this allocator.

### Layer 3 — Thread-Local Cache ⚡

The performance layer. Each thread gets its own stash of up to 8 blocks per size class stored as `thread_local` variables — no locking needed for normal operations.

- **Fast path** (cache hit) — pop from thread-local stash. Zero locks, zero contention.
- **Slow path** (cache miss) — take the global mutex, batch-fetch 4 blocks from the slab, release. One lock acquisition buys you 4 allocations.
- **On free** — push to thread-local stash. If stash exceeds limit, batch-return 4 blocks to the global slab under the lock.

This is exactly how tcmalloc works. "tc" literally stands for thread-cache.

---

## 📁 Project Structure

```text
memory-allocator/
├── include/
│   ├── alignment.hpp   # align_up, is_aligned, ASSERT_ALIGNED
│   ├── pool.hpp        # fixed-size block pool
│   ├── slab.hpp        # multi size-class slab allocator
│   └── cache.hpp       # thread-local cache layer
├── src/
│   ├── pool.cpp
│   ├── slab.cpp
│   └── cache.cpp
├── tests/
│   ├── test_pool.cpp
│   ├── test_slab.cpp
│   └── test_cache.cpp
└── bench/
    └── benchmark.cpp
```

---

## 🔨 Build & Run

Requires GCC 8+ and C++17.

### Run tests

```bash
# pool
g++ tests/test_pool.cpp src/pool.cpp -I include -std=c++17 -o test_pool
./test_pool

# slab
g++ tests/test_slab.cpp src/slab.cpp src/pool.cpp -I include -std=c++17 -o test_slab
./test_slab

# cache
g++ tests/test_cache.cpp src/cache.cpp src/slab.cpp src/pool.cpp -I include -std=c++17 -pthread -o test_cache
./test_cache
```

### Run benchmark

```bash
g++ bench/benchmark.cpp src/cache.cpp src/slab.cpp src/pool.cpp -I include -std=c++17 -pthread -o benchmark
./benchmark
```

---

## 📊 Benchmark Results

Averaged over 5 runs, 100k operations, single thread, Windows 11 / GCC 16.

| Pattern | Ours (ms) | malloc (ms) | Speedup |
|---|---:|---:|---:|
| uniform | 9.35 | 16.28 | **1.74x** |
| mixed | 9.12 | 8.05 | 0.88x |
| churn | 2.85 | 6.42 | **2.25x** |

**uniform** — consistent win. Same size every time, the pool goes straight to the right free-list head, no searching, no coalescing.

**mixed** — malloc wins slightly. Windows' UCRT allocator already has internal per-thread caches, and cycling across 6 size classes dilutes our cache effectiveness. On Linux with glibc malloc the gap closes.

**churn** — biggest win. Freed blocks return to the thread-local cache instantly. The next allocation pulls them right back without touching the global pool at all. malloc has to do bookkeeping every time.

---

## ⚠️ Limitations & Future Work

This is an educational implementation, not a production allocator. Known limitations:

- **No slab chaining** — each pool has a fixed block count. If exhausted, returns `nullptr` instead of growing.
- **Max allocation size is 1024 bytes** — anything larger falls through to the caller.
- **Windows benchmark** — Windows' UCRT malloc is exceptionally well optimized. Linux numbers would show a clearer win on mixed patterns.
- **No NUMA awareness** — on multi-socket systems, memory locality matters and this allocator ignores it.

### Things worth adding

- Slab chaining — allocate a new slab when current one is exhausted
- A fallback to `malloc` for oversized requests instead of returning `nullptr`
- Linux benchmarks for a fairer comparison against glibc malloc

---

## 📚 References

- [The Slab Allocator — Jeff Bonwick (1994)](https://www.usenix.org/legacy/publications/library/proceedings/bos94/bonwick.html) — the original paper that introduced slab allocation, now used in Linux
- [TCMalloc Design](https://google.github.io/tcmalloc/design.html) — Google's thread-caching malloc, direct inspiration for the cache layer
- [Linux SLUB Allocator](https://www.kernel.org/doc/html/latest/mm/slub.html) — the kernel's current slab implementation
