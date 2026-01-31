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

## Migration Results

Since we the differnce in pinning threads is neglible I will only be focusing on non pinned

Timeout reached after 10 minutes

```json
Records=2170787
bytes=43638995
time=600.001s
throughput=3617.97 rec/s
```

We can clearly see an increase in through put of roughly 5% for unpinned. We are noew processing about 43MB in ten minutes.

This is still way to slow.

we should be able to achieve even faster behavior by using the **try_emplace** method to insert elements.
this is usually faster than using the operator[] becuase it will use the values provided to create an element instead of making a copy or move operaiotn,.
Here are the results

```json
Timeout reached after 10 minutes

Records=2257123
bytes=45373800
time=600.001s
throughput=3761.87 rec/s
```

We can see a very slight increase in the number of records.

even after this change we can still see we are spending much time in the **addOrder** function

now we can see we are spending much of our time in **matchOrder** where we are reserving memory for trades

```cpp
  trades.reserve(asks.size() + bids.size());
```

likely here we need to use some ort of custom memory allocator

## Migration wave 2

Once the inital bottle neck discovered by the profiler of using a std::map was rectified the intial hypothesis of the calls to **reserve** being a potential bottle neck resurfaced.

After making the follwoing changes, preallocating memory for trades in the constructor of our OrderBook and moving from a std::list to a std::deque to represent our trades we were finally able to get this

```json
Done.
Records=53418218
 bytes=1073752675
 time=528.944s
 throughput=100990 rec/s
```

This means we needed the follwing improvements in our single threade approach to beat our timelimit of 10 minutes but still be far off from our goal of two minutes.

Preallocaing the memory for the trades was such a major improvenet becuase on each call to
**matchOrder** we were reallocating a **new vector** of size **asks +bids** which depending on the size of the maps could be very large and from the cpp standard the complexity is "linear in the size() of the container". This may be fast indivdually but called repjeatly can cause some performance degradation preallocaitng the memory and clearing after use woudl give us a signifacnt increase because we not only elmiate the need to allocate new but the memroy we would need we already have and can use readily.

Migrating from std::list to std::deque, may at first seem not needed because they both have O(1) insertion and deletion to the front and back of the containers. However, the deque will have a slight edge when it comes to memory locality, since deque is stored as **contiguous blocks of memory** and list is stored as **linked list** with node ptrs the localilty for the deque will be slightly better resulting in better performance since the memory needed will liekley be closer for use.

- Change from a std::map to a std::unordered_map the man benefit here is that instead of reling on a red-black tree for looks ups we were now using a hash table that has a O(1) lookup time
- Move the trades form **matchOrder** to a member of the **Orderbook** class and preallocate a sufficent amount of memory in our case 10,000 elements instead of calling **reserve** whenver we want to create a trades which ended up being very costly
- Moved from std::list to std::deque to represent Orders becuase of the O(1) for insertions and deletions
