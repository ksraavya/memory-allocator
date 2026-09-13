#include "../include/pool.hpp"
#include "../include/alignment.hpp"
#include <iostream>

#ifdef _WIN32
    #include <cstdlib>
    #define GET_MEMORY(size) malloc(size)
    #define FREE_MEMORY(ptr, size) free(ptr)
#else
    #include <sys/mman.h>
    #define GET_MEMORY(size) mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
    #define FREE_MEMORY(ptr, size) munmap(ptr, size)
#endif

Pool::Pool(size_t block_size, size_t block_count) {
    this->block_size = block_size;
    this->block_count = block_count;

    // allocate enough for block_count blocks plus one extra block for alignment padding
    size_t total = block_size * block_count + block_size;
    char* raw = static_cast<char*>(GET_MEMORY(total));
    this->memory = raw;
    this->total_size = total; 

    // snap start address forward to block_size alignment
    uintptr_t addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned_addr = align_up(addr, block_size);
    char* ptr = reinterpret_cast<char*>(aligned_addr);

    // link all block_count blocks into free list
    FreeBlock* head = reinterpret_cast<FreeBlock*>(ptr);
    FreeBlock* curr = head;

    for(size_t i = 1; i < block_count; i++) {
        FreeBlock* next = reinterpret_cast<FreeBlock*>(ptr + i * block_size);
        curr->next = next;
        curr = next;
    }
    curr->next = nullptr;
    free_list = head;
}

Pool::~Pool() {
    FREE_MEMORY(memory, total_size);
}

void* Pool::allocate() {
    if(free_list == nullptr) {
        return nullptr;
    }
    void* ptr = free_list;
    free_list = free_list->next;
    ASSERT_ALIGNED(ptr, block_size);
    return ptr;
}

void Pool::deallocate(void* ptr) {
    FreeBlock* newHead = reinterpret_cast<FreeBlock*>(ptr);
    newHead->next = free_list;
    free_list = newHead;
}