#include "../include/cache.hpp"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <vector>

static const size_t OPS = 100000;

template<typename Func>
double time_ms(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// ---- pattern 1: uniform ----
void bench_uniform_ours(CacheAllocator& alloc) {
    static const size_t BATCH = 64;  // allocate and free in small batches
    std::vector<void*> ptrs(BATCH);
    for(size_t i = 0; i < OPS; i += BATCH) {
        for(size_t j = 0; j < BATCH; j++)
            ptrs[j] = alloc.allocate(32);
        for(size_t j = 0; j < BATCH; j++)
            alloc.deallocate(ptrs[j], 32);
    }
}

void bench_uniform_malloc() {
    std::vector<void*> ptrs(OPS);
    for(size_t i = 0; i < OPS; i++)
        ptrs[i] = malloc(32);
    for(size_t i = 0; i < OPS; i++)
        free(ptrs[i]);
}

// ---- pattern 2: mixed ----
static const size_t MIXED_SIZES[] = {8, 16, 32, 64, 128, 256};
static const size_t NUM_MIXED = 6;

void bench_mixed_ours(CacheAllocator& alloc) {
    static const size_t BATCH = 64;
    std::vector<void*> ptrs(BATCH);
    for(size_t i = 0; i < OPS; i += BATCH) {
        for(size_t j = 0; j < BATCH; j++)
            ptrs[j] = alloc.allocate(MIXED_SIZES[(i + j) % NUM_MIXED]);
        for(size_t j = 0; j < BATCH; j++)
            alloc.deallocate(ptrs[j], MIXED_SIZES[(i + j) % NUM_MIXED]);
    }
}


void bench_mixed_malloc() {
    static const size_t BATCH = 64;
    std::vector<void*> ptrs(BATCH);
    for(size_t i = 0; i < OPS; i += BATCH) {
        for(size_t j = 0; j < BATCH; j++)
            ptrs[j] = malloc(MIXED_SIZES[(i + j) % NUM_MIXED]);
        for(size_t j = 0; j < BATCH; j++)
            free(ptrs[j]);
    }
}

// ---- pattern 3: churn ----
void bench_churn_ours(CacheAllocator& alloc) {
    for(size_t i = 0; i < OPS; i++) {
        void* p = alloc.allocate(32);
        alloc.deallocate(p, 32);
    }
}

void bench_churn_malloc() {
    for(size_t i = 0; i < OPS; i++) {
        void* p = malloc(32);
        free(p);
    }
}

int main() {
    CacheAllocator alloc;

    std::cout << "\n=== Memory Allocator Benchmark ===\n";
    std::cout << "Operations per test: " << OPS << "\n\n";

    std::cout << std::left;
    std::cout.width(20); std::cout << "Pattern";
    std::cout.width(15); std::cout << "Ours (ms)";
    std::cout.width(15); std::cout << "malloc (ms)";
    std::cout << "Speedup\n";
    std::cout << std::string(55, '-') << "\n";

    auto print_row = [](const char* name, double ours, double sys) {
        std::cout.width(20); std::cout << name;
        std::cout.width(15); std::cout << ours;
        std::cout.width(15); std::cout << sys;
        std::cout << sys / ours << "x\n";
    };

    double ours, sys;

    ours = time_ms([&]{ bench_uniform_ours(alloc); });
    sys  = time_ms([&]{ bench_uniform_malloc(); });
    print_row("uniform", ours, sys);

    ours = time_ms([&]{ bench_mixed_ours(alloc); });
    sys  = time_ms([&]{ bench_mixed_malloc(); });
    print_row("mixed", ours, sys);

    ours = time_ms([&]{ bench_churn_ours(alloc); });
    sys  = time_ms([&]{ bench_churn_malloc(); });
    print_row("churn", ours, sys);

    std::cout << "\n(speedup > 1.0x means our allocator is faster)\n";
    return 0;
}