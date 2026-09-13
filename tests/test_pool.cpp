#include "../include/pool.hpp"
#include <iostream>
#include <cassert>

int main() {
    // create a pool of 10 blocks, each 32 bytes
    Pool pool(32, 10);    

    // allocate 10 blocks
    void* ptrs[10];
    for(int i = 0; i < 10; i++) {
        ptrs[i] = pool.allocate();
        assert(ptrs[i] != nullptr);
        std::cout << "allocated block " << i << " at " << ptrs[i] << "\n";
    }

    // pool should be exhausted now
    void* shouldBeNull = pool.allocate();
    assert(shouldBeNull == nullptr);
    std::cout << "pool exhausted correctly\n";

    // free 5 blocks
    for(int i = 0; i < 5; i++) {
        pool.deallocate(ptrs[i]);
    }

    // should be able to allocate 5 more
    for(int i = 0; i < 5; i++) {
        void* p = pool.allocate();
        assert(p != nullptr);
        std::cout << "reallocated block at " << p << "\n";
    }

    std::cout << "all tests passed!\n";
    return 0;
}