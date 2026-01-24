# Single-Threaded Matching Engine Evaluation

## Experimental Constraints

All experiments were run on a Windows machine with the following specifications:

- Processor: AMD Ryzen 5 3600X (6 cores, 3.80 GHz)
- RAM: 32 GB
- Storage: 932 GB NVMe SSD (WDS100T3X0C-00SJG0)
- Graphics Card: NVIDIA GeForce RTX 3060 (12 GB)

To simulate a flood of market order traffic, we streamed 1 GB of order data from a local file into the matching engine.
Each run was time-boxed to 10 minutes to ensure comparability across configurations.

## Target Throughput

The primary goal of this application is to process 1 GB of order data in under 2 minutes.
This implies a sustained throughput requirement of approximately:
8.33 MB/s (1 GB / 120 s)

Assuming:

- A new order size of 32 bytes, this translates to roughly 260k orders/sec
- A cancel-heavy workload with 8-byte messages would require up to ~1.04M operations/sec

These values establish the target range the engine must eventually support.

## Data Structures

The order book is structured using two layers:
Price-level books for bids and asks:

```cpp
std::map<Model::Price, Orders, std::less<Model::Price>> asks;
```

This structure maintains price ordering via the comparator.
Time priority is handled separately within each price level.
Order lookup table, keyed by order ID, used for fast cancellation and modification:

```cpp
orders[order.getOrderId()] = orderPtr;
```

## Performance Results

### No Core Pinning

The process was allowed to run freely under the OS scheduler.

```json
Timeout reached after 10 minutes
Records processed: 1,994,767
Bytes processed: 40,101,591 (~40 MB)
Time: 600 s
Throughput: 3,324.61 records/sec
```

### Core Affinity (Pinned to a Single Core)

The process was pinned to a single physical core to reduce scheduler migration and improve cache locality.

```json
Timeout reached after 10 minutes
Records processed: 2,060,072
Bytes processed: 41,413,409 (~41 MB)
Time: 600.001 s
Throughput: 3,433.45 records/sec
```

Pinning the thread resulted in a ~3% improvement in throughput, indicating that scheduler migration is not a dominant bottleneck for this workload.

## Profiling Findings

CPU profiling revealed that approximately 63% of total execution time is spent in the following operation:

```cpp
orders[order.getOrderId()] = orderPtr;
```

This indicates that insertion into the order lookup map is the primary performance bottleneck.
Notably, the profiling data shows negligible time spent on memory reservation for trade objects, invalidating the initial hypothesis that repeated reserve calls were a significant performance issue.

### Analysis

The observed behavior is consistent with **std::map** being implemented as a **red-black tree.**

Each insertion requires:

- Heap allocation of a new node
- Pointer chasing during tree traversal
- Rebalancing operations
- operator[] performs a lookup followed by insertion on a miss
- Storing shared_ptr values introduces additional atomic reference count updates

While the theoretical insertion complexity is O(log n), the constant factors—allocator overhead and cache inefficiency—dominate at this scale.

## Improvement Plan

To address the primary bottleneck:

- Replace **std::map** used for order lookup with **std::unordered_map**
- Average O(1) insertion and lookup
- Improved cache locality
- Pre-allocate the hash table using reserve to avoid rehashing
- Replace operator[] with try_emplace to avoid redundant work
- Re-evaluate the use of shared_ptr in hot paths

These changes are expected to significantly reduce CPU time spent on order insertion and move the system closer to the required throughput targets.
