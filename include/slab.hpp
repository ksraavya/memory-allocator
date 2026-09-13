#pragma once

#include "pool.hpp"
#include <cstddef>

// the 8 size classes supported by the allocator
static const size_t SIZE_CLASSES[] = {8, 16, 32, 64, 128, 256, 512, 1024};
static const size_t NUM_CLASSES = 8;
static const size_t BLOCKS_PER_POOL = 64; // how many blocks each pool holds

class SlabAllocator {
private:
    Pool pools[NUM_CLASSES]; // one pool per size class

    // given a requested size, returns the index into SIZE_CLASSES
    // e.g. size_to_class(10) returns 1 (the 16-byte class)
    // e.g. size_to_class(64) returns 3 (the 64-byte class)
    // returns NUM_CLASSES if size is too large
    size_t size_to_class(size_t size);

public:
    SlabAllocator();
    void* allocate(size_t size);
    void deallocate(void* ptr, size_t size);
};