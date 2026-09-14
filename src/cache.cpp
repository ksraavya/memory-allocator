#include "../include/cache.hpp"
#include <cstddef>

// per-thread cache 
struct ThreadCache {
    void*  blocks[NUM_CLASSES][MAX_CACHE];
    size_t count[NUM_CLASSES];
};

static thread_local ThreadCache local_cache = {};

CacheAllocator::CacheAllocator() {}

size_t CacheAllocator::size_to_class(size_t size) {    
    for(int i=0; i<NUM_CLASSES; i++) {
        if(SIZE_CLASSES[i] >= size) {
            return i;
        }
    }
    return NUM_CLASSES;
}

void* CacheAllocator::allocate(size_t size) {    
    size_t idx = size_to_class(size);
    if(idx == NUM_CLASSES) return nullptr;

    //fast path using local_cache
    if(local_cache.count[idx] > 0) {
        local_cache.count[idx]--;
        return local_cache.blocks[idx][local_cache.count[idx]];
    }

    //slow path taking global mutex and refilling cache
    {
        std::lock_guard<std::mutex> guard(global_mutex);
        for(size_t i = 0; i < BATCH_SIZE; i++) {
            void* block = slab.allocate(SIZE_CLASSES[idx]);
            if(block == nullptr) break;
            local_cache.blocks[idx][local_cache.count[idx]] = block;
            local_cache.count[idx]++;
        }
    }

    if(local_cache.count[idx] > 0) {
        local_cache.count[idx]--;
        return local_cache.blocks[idx][local_cache.count[idx]];
    }

    return nullptr;
}

void CacheAllocator::deallocate(void* ptr, size_t size) {    
    size_t idx = size_to_class(size);
    if(idx == NUM_CLASSES) return;

    local_cache.blocks[idx][local_cache.count[idx]] = ptr;
    local_cache.count[idx]++;

    if(local_cache.count[idx] >= MAX_CACHE) {
        std::lock_guard<std::mutex> guard(global_mutex);
        for(size_t i = 0; i < BATCH_SIZE; i++) {
            local_cache.count[idx]--;
            slab.deallocate(
                local_cache.blocks[idx][local_cache.count[idx]], 
                SIZE_CLASSES[idx]
            );
        }
    }
}