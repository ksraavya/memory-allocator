# Memory Allocator

A from-scratch C++ memory allocator exploring low-level memory management, allocation strategies, alignment, and performance.

## Overview

This project starts with a fixed-size memory pool and free-list allocator, and progressively extends toward a slab allocator with multiple size classes and thread-local caches.

## Goals

- Understand raw memory management and allocation
- Implement free-list and slab-based allocation
- Explore alignment, fragmentation, and caching
- Benchmark against `malloc`

## Tech Stack

- C++
- CMake