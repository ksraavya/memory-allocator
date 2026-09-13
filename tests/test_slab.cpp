#include "../include/slab.hpp"
#include <iostream>
#include <cassert>

int main() {
    SlabAllocator slab;

    // allocate various sizes and verify they come back non-null
    void* p1 = slab.allocate(5);   // should use 8-byte pool
    assert(p1 != nullptr);
    std::cout << "allocated 5 bytes at " << p1 << "\n";

    void* p2 = slab.allocate(16);  // should use 16-byte pool exactly
    assert(p2 != nullptr);
    std::cout << "allocated 16 bytes at " << p2 << "\n";

    void* p3 = slab.allocate(50);  // should use 64-byte pool
    assert(p3 != nullptr);
    std::cout << "allocated 50 bytes at " << p3 << "\n";

    void* p4 = slab.allocate(1024); // should use 1024-byte pool
    assert(p4 != nullptr);
    std::cout << "allocated 1024 bytes at " << p4 << "\n";

    // too large — should return nullptr
    void* p5 = slab.allocate(2000);
    assert(p5 == nullptr);
    std::cout << "oversized allocation correctly returned nullptr\n";

    // deallocate and reallocate — should work cleanly
    slab.deallocate(p1, 5);
    void* p6 = slab.allocate(5);
    assert(p6 != nullptr);
    std::cout << "reallocated 5 bytes at " << p6 << "\n";

    // p6 should be the same address as p1 since it was just freed
    assert(p6 == p1);
    std::cout << "address matches — free list working correctly\n";

    std::cout << "all slab tests passed!\n";
    return 0;
}