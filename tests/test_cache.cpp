#include "../include/cache.hpp"
#include <iostream>
#include <cassert>
#include <thread>

void single_thread_test() {
    CacheAllocator alloc;

    // basic allocate and deallocate
    void* p1 = alloc.allocate(16);
    assert(p1 != nullptr);
    std::cout << "allocated 16 bytes at " << p1 << "\n";

    void* p2 = alloc.allocate(64);
    assert(p2 != nullptr);
    std::cout << "allocated 64 bytes at " << p2 << "\n";

    // too large
    void* p3 = alloc.allocate(2000);
    assert(p3 == nullptr);
    std::cout << "oversized correctly returned nullptr\n";

    alloc.deallocate(p1, 16);
    alloc.deallocate(p2, 64);

    // reallocate — should come from cache now
    void* p4 = alloc.allocate(16);
    assert(p4 == p1);  // same address, came from cache
    std::cout << "cache working — got same block back\n";

    std::cout << "single thread tests passed\n";
}

void multi_thread_test() {
    CacheAllocator alloc;
    
    auto worker = [&alloc](int id) {
        for(int i = 0; i < 10; i++) {
            void* p = alloc.allocate(32);
            assert(p != nullptr);
            alloc.deallocate(p, 32);
        }
        std::cout << "thread " << id << " done\n";
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "multi thread tests passed\n";
}

int main() {
    single_thread_test();
    multi_thread_test();
    std::cout << "all cache tests passed!\n";
    return 0;
}