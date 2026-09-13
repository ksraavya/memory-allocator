#pragma once

#include <cstdint>
#include <cassert>

// Rounds 'addr' up to the nearest multiple of 'alignment'
// alignment MUST be a power of 2 (8, 16, 32, 64...)
inline uintptr_t align_up(uintptr_t addr, std::size_t alignment) {
    return (addr + alignment - 1) & ~(alignment - 1);
}

// Checks if a given address is aligned to 'alignment'
inline bool is_aligned(void* ptr, std::size_t alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

// Asserts alignment in debug builds
#define ASSERT_ALIGNED(ptr, alignment) assert(is_aligned(ptr, alignment) && "pointer is not aligned!")