#include "../include/slab.hpp"
#include <cstddef>

SlabAllocator::SlabAllocator() 
    : pools{
        Pool(SIZE_CLASSES[0], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[1], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[2], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[3], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[4], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[5], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[6], BLOCKS_PER_POOL),
        Pool(SIZE_CLASSES[7], BLOCKS_PER_POOL)
    }
{}

size_t SlabAllocator::size_to_class(size_t size) {
    for(int i=0; i<NUM_CLASSES; i++) {
        if(SIZE_CLASSES[i] >= size) {
            return i;
        }
    }
    return NUM_CLASSES;
}

void* SlabAllocator::allocate(size_t size) {    
    size_t idx = size_to_class(size);
    if(idx == NUM_CLASSES) return nullptr;
    return pools[idx].allocate();    
}

void SlabAllocator::deallocate(void* ptr, size_t size) {    
    int idx = size_to_class(size);
    if(idx == NUM_CLASSES) return;
    pools[idx].deallocate(ptr);
}