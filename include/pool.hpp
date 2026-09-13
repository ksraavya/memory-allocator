#pragma once

#include <cstdint>

class Pool {
private:
    struct FreeBlock {
        FreeBlock* next;
    };

    void* memory;                                   // the raw slab
    std::size_t block_size;                         // size of each block
    std::size_t block_count;                        // total number of blocks
    std::size_t total_size;                         // total size
    FreeBlock* free_list;                           // head of the free list

public:
    Pool(std::size_t block_size, std::size_t block_count);
    ~Pool();
    void* allocate();
    void deallocate(void* ptr);
};