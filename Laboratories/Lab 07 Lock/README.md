<script type="text/javascript"
    src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
</script>
<script type="text/x-mathjax-config">
    MathJax.Hub.Config({
    tex2jax: {inlineMath: [['$', '$']]},
    "HTML-CSS": {
        fonts: ["Neo-Euler"],
    },
    messageStyle: "none"
  });
</script>

# Lab 07 Lock

## Introduction

In this lab, we will optimize the memory allocator and buffer cache of the xv6 operating system to improve performance on multi-core systems. The original implementations used coarse-grained locking strategies that severely limited concurrency, leading to performance bottlenecks. We will replace these with fine-grained locking techniques, allowing multiple cores to operate concurrently without contention.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout lock
```

## Optimizing Memory Allocator

A key challenge in a multi-core operating system is managing contention for shared resources. The original xv6 memory allocator (`kalloc`) used a single, global freelist protected by a single lock. This design becomes a significant performance bottleneck on multi-core systems, as all CPUs must compete for this one lockinflammation for every memory allocation and deallocation, leading to high lock contention and serialized execution.

To address this, we re-architected the memory allocator to use a per-CPU freelist design, supplemented by a work-stealing mechanism. This approach dramatically reduces lock contention and improves the overall scalability of the kernel.

### Per-CPU Freelist

The core of the new design is to give each CPU core its own private pool of free memory pages. The global `kmem` structure was transformed into an array, where each element corresponds to a CPU core:

```c
struct {
  struct spinlock lock;
  struct run *freelist;
  int count;
} kmem[NCPU];
```

* **`kmem[i].lock`**: A dedicated spinlock that protects only the freelist of CPU `i`.
* **`kmem[i].freelist`**: A pointer to the head of the linked list of free pages for CPU `i`.
* **`kmem[i].count`**: The number of free pages available to CPU `i`.

During initialization (`kinit`), each per-CPU structure is initialized with its own lock. `freerange` is modified to populate the freelist of the specific CPU it is running on.

### Allocation and Deallocation Path

With this new structure, the common-case allocation (`kalloc`) and deallocation (`kfree`) operations become highly parallel:

* **`kalloc()`**: When a process running on CPU `i` needs a page, `kalloc` first attempts to acquire the lock for and allocate from `kmem[i]`'s local freelist, if it is empty, it will then call `ksteal` to attempt to steal pages from another CPU's freelist. When `ksteal` doesn't find any available pages, it will return 0, indicating that no pages are available for allocation.
* **`kfree()`**: When a process on CPU `i` frees a page, that page is returned to the local freelist of `kmem[i]`.

Since each CPU primarily interacts with its own locked freelist, contention is eliminated as long as each CPU has a sufficient supply of free pages. CPUs can allocate and free memory concurrently without waiting for one another.

```c
// In kalloc()
push_off();
cpu = cpuid();
pop_off();

acquire(&kmem[cpu].lock);
r = kmem[cpu].freelist;
// ...
release(&kmem[cpu].lock);
```

### Work-Stealing Mechanism

A per-CPU design introduces a new problem: what if one CPU's freelist becomes empty while other CPUs have plenty of free pages? To solve this imbalance, we implemented a work-stealing mechanism in the `ksteal` function.

If `kalloc` finds its local freelist empty, it releases its local lock and calls `ksteal` to "steal" a batch of free pages from another CPU.

* `ksteal(stealer_cpu)` iterates through all other CPUs (`victim_cpu`).
* It attempts to acquire the lock of a victim CPU.
* If the victim has pages to spare, `ksteal` removes a small batch of pages (`STEAL_AMOUNT`) from the victim's freelist.
* It releases the victim's lock.
* Finally, it acquires the stealer's lock and adds the stolen pages to its own freelist.

This ensures that memory pages can be redistributed dynamically across cores, preventing a single core from starving while others are idle.

```c
// In kalloc(), on cache miss
if(!r){
  // No free pages available, try to steal from another CPU.
  release(&kmem[cpu].lock);

  ksteal(cpu);

  acquire(&kmem[cpu].lock);
  // Re-attempt allocation from the now possibly refilled local freelist
}
```

### Synchronization and Deadlock Avoidance

The stealing mechanism introduces a new synchronization challenge: a CPU might need to hold two locks simultaneously (the victim's and its own). The initial implementation of `ksteal` acquired the victim's lock, moved pages, released the victim's lock, and then acquired its own lock. This "lock-coupling" approach avoids holding two locks at once, thereby preventing a classic AB-BA deadlock scenario where two CPUs might try to steal from each other simultaneously and wait indefinitely for the other's lock.

To address this, we break the stealing operation into two phases:

1\. **Steal Phase**: Takes the victim's lock, removes a batch of pages, and releases the victim's lock.
2\. **Reclaim Phase**: Acquires the stealer's lock and adds the stolen pages to its freelist.

```c
int
ksteal(int stealer)
{
  struct run *start, *end;  // Start and end of stolen freelist.
  int cpu, total_stolen, stolen;

  total_stolen = 0;

  for(cpu = (stealer + 1) % NCPU; cpu != stealer; cpu = (cpu + 1) % NCPU){
    acquire(&kmem[cpu].lock);
    start = kmem[cpu].freelist;
    end = start;
    if(!start){
      release(&kmem[cpu].lock);
      continue; // No pages to steal from this CPU.
    }

    // Find the end of the freelist to steal.
    stolen = 1;
    while(end->next && stolen + total_stolen < STEAL_AMOUNT){
      stolen++;
      end = end->next;
    }

    // Remove the stolen pages from the original CPU's freelist.
    kmem[cpu].freelist = end->next;
    kmem[cpu].count -= stolen;
    release(&kmem[cpu].lock);

    total_stolen += stolen;

    // Add the stolen pages to the stealer's freelist.
    acquire(&kmem[stealer].lock);
    if(!kmem[stealer].freelist){
      // Set the stealer's freelist to the stolen freelist.
      kmem[stealer].freelist = start;
      end->next = 0;
    } else {
      // Link the stolen pages to the end of the stealer's freelist.
      end->next = kmem[stealer].freelist;
      kmem[stealer].freelist = start;
    }
    kmem[stealer].count += stolen;
    release(&kmem[stealer].lock);

    if(total_stolen >= STEAL_AMOUNT)
      break; // Enough pages stolen.
  }

  return total_stolen;
}
```

## Optimizing Buffer Cache

The original xv6 buffer cache (`bcache`) relied on a single global lock to protect a simple LRU linked list of all buffers. This design, while simple, forces all block cache operations to be serialized, creating a major performance bottleneck in multi-core environments where file system access is frequent. To significantly improve concurrency and performance, we redesigned the buffer cache by replacing the global lock with a fine-grained locking strategy based on a hash table.

This optimization involved three key components: a **hash table with per-bucket locking**, a **bitmap freelist** for fast allocation, and a **CLOCK algorithm** for efficient buffer replacement.

### Hash Table with Per-Bucket Locks

To allow concurrent access to different blocks, we replaced the single linked list with a hash table. The hash table partitions the buffer cache into `NBUCKETS` (a prime number, 13) buckets.

* **Partitioning**: Each buffer is mapped to a bucket based on a hash of its `(dev, blockno)` identifier. This distributes access across different buckets.
* **Per-Bucket Locking**: Instead of one global lock, we introduced an array of `NBUCKETS` spinlocks (`bcache.bucket_locks`). Each lock protects a single bucket's linked list. This allows threads operating on different buckets to proceed in parallel, dramatically reducing lock contention.

The `bget` operation now first computes the hash to identify the correct bucket, acquires only that bucket's lock, and then searches the short linked list within that bucket. This transforms the lock scope from a global bottleneck to a highly localized, short-duration hold.

```c
// In bget(), when searching the cache:
hashval = hash(dev, blockno);
acquire(&bcache.bucket_locks[hashval]);
bufidx = find(dev, blockno);
if(bufidx >= 0) {
    // Cache hit. Only one bucket lock was held.
    // ...
    release(&bcache.bucket_locks[hashval]);
    // ...
}
```

### Bitmap Freelist for Fast Allocation

To quickly find an available `buf` structure when a block is not in the cache, we replaced the O(N) linear scan of the original LRU list with a bitmap freelist.

* **Data Structure**: A single 32-bit integer (`bcache.freelist`) is used, where each bit corresponds to the state of one of the `NBUF` buffers (0 for free, 1 for used).
* **O(1) Allocation**: The `allocbuf` function now performs a fast bitwise scan on this bitmap to find the first free buffer. This is significantly more efficient than iterating through a list or array, especially as the number of buffers grows. The operation is protected by the corresponding bucket lock to ensure atomicity during the cache-miss path.

### CLOCK Algorithm for Buffer Eviction

The original LRU replacement policy was tightly coupled with the global lock. We replaced it with the **CLOCK algorithm**, an efficient approximation of LRU that is well-suited for a concurrent environment.

**Mechanism**:

1. Each `struct buf` has a `referenced` bit.
2. A global "clock hand" (`bcache_clock.clock`) points to the next candidate for eviction.
3. When a buffer is accessed (a cache hit in `bget`), its `referenced` bit is set to 1.

**Eviction Process**: When a buffer needs to be evicted, the CLOCK algorithm, protected by its own `bcache_clock.lock`, scans from the current clock hand position:

* If it finds a buffer with `refcnt >= 0`, it skips to the next buffer.
* If it finds a buffer with `refcnt == 0` and `referenced == 1`, it gives the buffer a "second chance" by clearing the `referenced` bit to 0 and continues scanning.
* If it finds a buffer with `refcnt == 0` and `referenced == 0`, it selects this buffer for eviction.

If it scans through all buffers for two full rotations without finding a victim, it will break out of the loop, indicating that all buffers are in use and no eviction is possible.

This strategy ensures that frequently used buffers (whose `referenced` bit is often set to 1) are less likely to be evicted, effectively preventing the "cache thrashing" problem observed in tests like `bcachetest`'s `test1`, all without the need for a costly, globally-locked linked list.

```c
// Eviction logic in bget()
acquire(&bcache_clock.lock);
for(n = 0; n < 2 * NBUF; n++){
  bcache_clock.clock++;
  if(bcache_clock.clock >= NBUF)
    bcache_clock.clock = 0;
  buf = &bcache.buf[bcache_clock.clock];
  if(buf->refcnt == 0 && !buf->referenced){
    // Found a buffer to evict.
    bufidx = bcache_clock.clock;
    release(&bcache_clock.lock);
    erase(buf->dev, buf->blockno);
    buf->dev = dev;
    buf->blockno = blockno;
    buf->valid = 0;
    buf->refcnt = 1;
    buf->referenced = 1;
    insert(dev, blockno, bufidx);
    release(&bcache.bucket_locks[hashval]);
    acquiresleep(&buf->lock);
    return buf; 
  }
  if(buf->refcnt == 0){
    // Found a recently referenced buffer, reset its referenced flag.
    buf->referenced = 0;
  }
}
panic("bget: no buffers");
```

By combining these three optimizations, the buffer cache is transformed from a serial bottleneck into a highly concurrent and performant system component. The contention on any single lock is minimized, allowing the xv6 kernel to scale effectively on multi-core processors and handle demanding file system workloads efficiently.

## Relevant Files

* `kernel/kalloc.c`: Contains the new memory allocator implementation, including the per-CPU freelist and work-stealing mechanism.
* `kernel/bio.c`: Contains the primary implementation of the buffer cache, including the hash table, per-bucket locks, bitmap freelist, and the CLOCK replacement algorithm.
* `kernel/buf.h`: Contains the modified `struct buf` definition, removing the `prev` and `next` pointers, and adding the `referenced` field.

Of course. Here is the consolidated summary section for both the Memory Allocator and Buffer Cache optimizations, following the structure you provided.

## Key Learning Outcomes

This lab provided deep, practical experience in identifying and resolving performance bottlenecks in a multi-core kernel through advanced locking strategies.

* **Fine-Grained Locking**: Mastered the technique of replacing coarse, global locks with fine-grained locks (per-CPU and per-bucket) to increase parallelism. This demonstrated the direct trade-off between lock complexity and system scalability.
* **Concurrent Data Structure Design**: Learned to design and implement concurrent data structures suitable for a kernel environment, including a per-CPU freelist array and a lock-striped hash table.
* **Work-Stealing and Load Balancing**: Implemented a work-stealing scheduler for the memory allocator, providing insight into how modern systems dynamically balance resources across cores to prevent starvation and improve utilization.
* **Cache Replacement Algorithms**: Moved beyond simple LRU to implement the CLOCK algorithm, a widely used, efficient approximation of LRU. This highlighted the importance of choosing algorithms that are not only theoretically sound but also performant under high concurrency.
* **Deadlock Avoidance**: Gained practical experience in identifying and preventing deadlock, particularly when a single operation requires acquiring multiple locks. The principle of establishing a global lock acquisition order was a key takeaway.
* **Performance Analysis**: Learned to interpret performance metrics (like lock contention counts from `ntas`) to pinpoint bottlenecks and validate the effectiveness of optimizations. The dramatic difference in `bcachetest` execution time underscored the real-world impact of algorithmic choices in a concurrent setting.

## Challenges and Solutions

Implementing these optimizations involved overcoming several classic kernel development challenges:

* **Deadlock in `ksteal`**: The initial design for the memory stealer could lead to an AB-BA deadlock if two CPUs tried to steal from each other simultaneously.
* **Solution**: The implemented `ksteal` logic avoids this by never holding two per-CPU locks at once. It acquires the victim's lock, removes pages, releases the victim's lock, and only then acquires its own lock to add the stolen pages.

* **Deadlock in Buffer Eviction**: When replacing a buffer in `bget`, the chosen victim buffer might reside in a different hash bucket than the new block. This requires locking both the old and new buckets, creating a potential for deadlock.
* **Solution**: A strict lock ordering protocol was required. While not fully implemented in the final code, the principle is to always acquire locks for buckets with a lower index before acquiring locks for buckets with a higher index. Special care must be taken for the case where the old and new block hash to the same bucket. For this lab, serializing the eviction process with a single `bcache_clock.lock` simplified the logic and effectively prevented this specific deadlock scenario.

* **Atomic Cache Lookups and Allocation**: A critical race condition exists between checking if a block is in the cache and allocating a new buffer for it if it's not. If not atomic, two processes could simultaneously miss, leading to two buffers being allocated for the same block.
* **Solution**: This was solved by ensuring that the lock for the target hash bucket is held continuously throughout the entire process of searching the bucket and, upon a miss, inserting the newly allocated buffer entry into that bucket's list.

* **Cache Thrashing and Performance Degradation**: The initial, non-LRU replacement strategy (simply picking any unused buffer) caused severe performance degradation in `bcachetest`, leading to timeouts. The cache was unable to retain "hot" data when faced with a competing "cold" data scan.
* **Solution**: The CLOCK algorithm was implemented. By giving recently used buffers a "second chance" before eviction, it effectively differentiates between frequently accessed blocks and those from a one-time scan. This successfully mitigates cache thrashing and allows the performance-critical tests to pass.

## Conclusion

This lab was a crucial exercise in modernizing a monolithic kernel for the multi-core era. By transforming both the memory allocator and the buffer cache from single-lock, serialized systems into highly concurrent, fine-grained locked subsystems, we achieved a significant leap in performance and scalability. The process involved not just implementing new data structures like hash tables and bitmaps, but also reasoning deeply about concurrency primitives, deadlock, race conditions, and the practical performance implications of different algorithms. The final solution, employing per-CPU freelists with work-stealing and a lock-striped hash cache with a CLOCK replacement policy, reflects the sophisticated yet practical designs found in production operating systems today.
