# Multi-Threaded Lock-Based Matching Engine Evaluation

## Experimental Constraints

All experiments were run on a Windows machine with the following specifications:

- Processor: AMD Ryzen 5 3600X (6 cores, 3.80 GHz)
- RAM: 32 GB
- Storage: 932 GB NVMe SSD (WDS100T3X0C-00SJG0)
- Graphics Card: NVIDIA GeForce RTX 3060 (12 GB)
- Logical Processors: 12

Cache Hierarchy:

- L1: 384 KB
- L2: 3.0 MB
- L3: 32.0 MB

To simulate a flood of market order traffic, we streamed 1 GB of order data from a local file into the matching engine. Each run was time-boxed to 10 minutes to ensure comparability across configurations.

## Target Throughput

The primary goal of this application is to process 1 GB of order data in under 2 minutes. This implies a sustained throughput requirement of approximately:

**8.33 MB/s** (1 GB / 120 s)

Assuming:

- A new order size of 32 bytes, this translates to roughly **260k orders/sec**
- A cancel-heavy workload with 8-byte messages would require up to **~1.04M operations/sec**

These values establish the target range the engine must eventually support.

## Single-Threaded Baseline Performance

Before introducing concurrency, we established a single-threaded baseline using the optimized implementation detailed in the single-threaded evaluation. The final single-threaded configuration achieved:

```json
Records: 53,418,218
Bytes: 1,073,752,675 (~1 GB)
Time: 528.944s (~8.8 minutes)
Throughput: 100,990 rec/s
```

### Key Optimizations from Single-Threaded Implementation

The single-threaded baseline incorporates the following critical optimizations:

1. **std::unordered_map for order lookup** - O(1) average-case insertion/lookup instead of O(log n) with std::map
2. **Pre-allocated trade memory** - Eliminated repeated allocations by pre-allocating 10,000 trade slots in the OrderBook constructor and clearing after use
3. **std::deque for order storage** - Better memory locality compared to std::list while maintaining O(1) insertions/deletions
4. **try_emplace for insertions** - Avoided redundant copy/move operations compared to operator[]

These optimizations reduced the processing time from 10+ minutes (timeout) to approximately 8.8 minutes, representing a **10x improvement** over the initial naive implementation.

## Multi-Threading Strategy

### Data Partitioning Approach

Unlike the single-threaded approach where orders were processed sequentially from the file, the multi-threaded implementation must partition the workload across multiple worker threads. Initial strategies under consideration:

1. **Per-thread chunking** - Divide the 1 GB file into N equal chunks (where N = number of threads)
2. **Dynamic work stealing** - Implement a shared queue with threads pulling batches of orders
3. **Symbol-based partitioning** - Route orders to specific threads based on symbol hash (if applicable)

For this evaluation, we begin with **per-thread chunking** as it provides the simplest baseline for measuring lock contention overhead.

### Synchronization Strategy

The core challenge in multi-threading the matching engine is protecting shared data structures:

- **Order book price levels** (asks/bids maps)
- **Order lookup table** (orders unordered_map)
- **Trade output queue**

Initial synchronization approach:

- **std::shared_mutex** for read-heavy operations (order book queries)
- **std::scoped_lock** for write operations (order insertion, matching, cancellation)
- Evaluate granularity: single global lock vs. per-symbol locks vs. per-price-level locks

## Hypothesis

### Expected Performance Characteristics

1. **Lock Contention Overhead**: Even with optimized data structures, we expect to see throughput degradation due to lock contention. The severity will depend on:
   - Read/write ratio of operations
   - Critical section size
   - Number of concurrent threads

2. **Scaling Predictions**:
   - **Best case**: Near-linear scaling up to 6 threads (physical cores), with diminishing returns beyond due to hyperthreading overhead
   - **Realistic case**: 50-70% scaling efficiency due to lock contention, assuming write-heavy workload
   - **Worst case**: Performance degradation beyond 4 threads if critical sections are too large

3. **Bottleneck Hypothesis**: We predict the primary bottleneck will be:
   - **Write contention** on the order lookup table during order insertion
   - **Lock convoy effects** during matching operations when multiple threads compete for the same price levels
   - **Cache line ping-ponging** for frequently accessed price levels

### Optimization Roadmap

Based on anticipated bottlenecks, the following optimizations will be evaluated in order:

1. **Shared vs. Exclusive Locking**
   - Baseline: std::scoped_lock (exclusive) for all operations
   - Improvement: std::shared_lock for read operations, std::unique_lock for writes
   - Expected gain: 10-20% if read operations dominate

2. **Lock Granularity**
   - Baseline: Single global mutex protecting the entire order book
   - Improvement 1: Per-side locks (separate mutex for bids and asks)
   - Improvement 2: Per-price-level locks using a lock striping strategy
   - Expected gain: 30-50% by reducing false sharing

3. **Batch Processing**
   - Process orders in small batches within critical sections to amortize lock acquisition cost
   - Expected gain: 15-25% by reducing lock/unlock frequency

4. **Lock-Free Alternatives** (if lock-based approach proves insufficient)
   - Explore atomic operations for simple updates
   - Consider lock-free queues for trade output
   - This represents a fundamental architecture change and will be evaluated separately

## Performance Evaluation Plan

### Test Configurations

We will evaluate the following thread counts: 1, 2, 4, 6, 8, 12

For each configuration, we measure:

- Total throughput (records/sec)
- Processing time to complete 1 GB
- CPU utilization per core
- Lock contention metrics (if tooling available)

### Success Criteria

- **Minimum viable**: Process 1 GB in under 2 minutes with any thread count
- **Optimal scaling**: Achieve >70% scaling efficiency up to 6 threads
- **Contention target**: Less than 30% of CPU time spent waiting on locks

### Profiling Focus

Unlike the single-threaded evaluation where allocation overhead was the primary concern, multi-threaded profiling will focus on:

1. Time spent in lock acquisition/release
2. Cache miss rates on shared data structures
3. Thread scheduling and context switch overhead
4. Critical section execution time

## Initial Results

_This section will be populated with experimental data as tests are conducted._

### Configuration 1: Global Lock with std::scoped_lock

**Thread Count: [TBD]**

```json
[Results pending]
```

### Configuration 2: Shared/Exclusive Locks

**Thread Count: [TBD]**

```json
[Results pending]
```

## Analysis and Findings

_This section will contain analysis of results, comparison to single-threaded baseline, and identification of dominant performance factors._

## Conclusions and Next Steps

_This section will summarize whether lock-based concurrency can meet the 2-minute target and outline the path forward (further optimizations vs. architectural changes)._
