#pragma once

#include "slab.hpp"
#include <mutex>
#include <cstddef>

static const size_t MAX_CACHE  = 8;
static const size_t BATCH_SIZE = 4;

class CacheAllocator {
private:
    SlabAllocator slab;
    std::mutex global_mutex;

    // converts requested size to size class index
    // same logic as slab — kept here so cache layer is self contained
    size_t size_to_class(size_t size);

public:
    CacheAllocator();
    void* allocate(size_t size);
    void  deallocate(void* ptr, size_t size);
};