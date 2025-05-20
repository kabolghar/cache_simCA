# Cache Simulator

A C++ cache simulator that analyzes performance of different cache configurations under various memory access patterns.

## Features

- Direct Mapped and Fully Associative cache architectures
- Configurable parameters:
  - Cache size: 64KB
  - Line sizes: 16, 32, 64, 128 bytes
  - Associativity: 1, 2, 4, 8, 16 ways
- LRU replacement policy
- Six memory access patterns (sequential and random)

## Memory Access Patterns

1. `memGen1`: Sequential access across DRAM (64MB)
2. `memGen2`: Random access within 24KB
3. `memGen3`: Random access across DRAM
4. `memGen4`: Sequential access within 4KB
5. `memGen5`: Sequential access within cache size
6. `memGen6`: Sequential access by 32 bytes within 256KB

## Experiments

The simulator runs two types of experiments for each pattern:

1. Fixed sets (4) with varying line sizes (16B to 128B)
2. Fixed line size (64B) with varying ways (1 to 16)

Each experiment runs for 1M iterations and reports hit ratios.

## Build & Run

```bash
g++ -std=c++11 cache.cpp -o cache_simulator
./cache_simulator
```

## Cache Structure

- Two-dimensional vector (sets × ways)
- Each line contains: valid bit, tag, and LRU tracker
- DRAM size: 64MB
- Cache size: 64KB

