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

# Lab 09 MMAP

## Introduction

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout mmap
```

## Overview

In modern operating systems, `mmap` (memory map) is a powerful system call that provides a mechanism for sophisticated memory management. Its primary functions include enabling high-performance file I/O, facilitating memory sharing between processes, and allowing applications to exercise fine-grained control over their virtual address space. By mapping a file directly into a process's address space, `mmap` allows the file to be accessed with simple pointer operations, as if it were an in-memory array. This approach eliminates the overhead of repeated `read()` and `write()` system calls and avoids the extra data copying between kernel and user space, thus providing a significant performance advantage.

### `mmap` and `munmap` System Calls

The standard function signature for `mmap` is as follows:

```c
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
```

* **`addr`**: A suggested starting address for the mapping.
* **`len`**: The number of bytes to map into the address space.
* **`prot`**: The desired memory protection for the mapping (e.g., readable, writable).
* **`flags`**: Flags that control the nature of the mapping (e.g., shared or private).
* **`fd`**: The file descriptor of the file to be mapped.
* **`offset`**: The offset within the file where the mapping should begin.

The corresponding `munmap` call is used to remove a memory mapping:

```c
int munmap(void *addr, size_t len);
```

### Implemented Features

For this lab, we implemented a simplified but functional subset of the `mmap` and `munmap` features, focusing on the core functionality of memory-mapping files:

* **Kernel-Chosen Address**: The `addr` argument to `mmap` is assumed to be `NULL`, meaning the kernel is responsible for finding a suitable, unused virtual address range for the mapping.
* **Protection Flags**: The `prot` argument supports `PROT_READ`, `PROT_WRITE`, or a combination of both, controlling the access permissions of the mapped memory.
* **Mapping Flags**:
  * **`MAP_SHARED`**: Modifications made to the mapped memory region will be written back to the underlying file, typically when `munmap` is called or the process exits.
  * **`MAP_PRIVATE`**: Modifications are private to the process and will not be written back to the file. (While the flag is recognized, the implementation may not include full Copy-on-Write support).
* **File and Offset**: A valid file descriptor `fd` is required, but the `offset` is assumed to be zero, meaning all mappings start from the beginning of the file.
* **Lazy Allocation**: A key feature of our implementation is lazy page allocation. `mmap` itself does not allocate physical memory or read file data. Instead, physical pages are allocated and populated with file content on-demand within the page fault handler (`usertrap`) the first time a memory location in the mapped region is accessed.
* **Partial Unmapping**: The `munmap` implementation supports unmapping a region at the start, at the end, or the entire mapped area, but not punching a hole in the middle of a VMA.

## Virtual Memory Area (VMA)

To manage the memory regions created by `mmap`, we introduced the concept of a Virtual Memory Area (VMA) into the kernel. A VMA is a kernel data structure that describes a contiguous range of a process's virtual address space and its associated properties.

### The VMA Structure

We defined a `struct vma` to hold all the necessary information for a single memory mapping:

```c
struct vma {
  uint64 start;      // Start address of the VMA.
  uint64 length;     // Length of the VMA.
  int prot;          // Protection flags (read, write, execute).
  int flags;         // Flags (shared, private, etc.).
  struct file *file; // Associated file, if any.
};
```

This structure serves as the kernel's internal "contract" for each `mmap` call, tracking where the mapping lives in the virtual address space, its size, its access permissions, and a pointer to the file it represents.

A full-featured operating system might use a dynamic data structure like a linked list or a balanced tree to manage a variable number of VMAs per process. However, the xv6 kernel lacks a general-purpose, variable-size allocator for small kernel objects, so we opt for a simpler approach:

### Fixed Array in `struct proc`

A fixed-size array of `struct vma` is embedded directly within the per-process structure (`struct proc`). This avoids the need for dynamic kernel memory allocation for VMAs. A size of `NVMA` (16) was chosen, which is sufficient for the lab's test cases.

```c
// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct vma vmas[NVMA];       // Virtual Memory Areas
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
```

### Pre-defined Virtual Address Slots

Instead of searching for a free region in the vast virtual address space, we simplified address allocation by partitioning a large, dedicated segment of the virtual address space into fixed-size "slots".

* A large region from `MMAPBASE` to `MMAPTOP` is reserved exclusively for `mmap` allocations.
* This region is divided into `NVMA` equal-sized slots, each `MMAPSIZE` bytes long.
* When a process calls `mmap`, the kernel finds the first available VMA entry in the `p->vmas` array. The virtual address for this mapping is then deterministically calculated based on the index of that entry: `addr = MMAPBASE + (index * MMAPSIZE)`.

```c
// mmap allocations.
#define MMAPBASE 0x200000000L                  // Base address for mmap allocations.
#define MMAPTOP (MMAPBASE + 0x20000000L)       // Top address for mmap allocations.
#define MMAPSIZE ((MMAPTOP - MMAPBASE) / NVMA) // Maximum size of each mmap allocation.
```

This simplified design is reasonable because a 64-bit virtual address space is enormous compared to the physical memory available. Dedicating a large, high-address region for `mmap` does not practically limit the process's heap growth (managed by `sbrk`) while greatly simplifying the logic for finding an available address range. This strategy effectively trades a small amount of virtual address space flexibility for a significant reduction in implementation complexity.

### Simplified VMA Management

When `munmap` is called, the kernel can handle the required cases by simply adjusting the `start` and `length` fields of the existing VMA, or by clearing the VMA entry if the entire region is unmapped. Since `munmap` will not create a hole, a single VMA never needs to be split into two. This makes managing the fixed array straightforward, as we never need to allocate a new VMA to represent the second half of a split region.

Absolutely. Here is the report section detailing the kernel-side implementation of `mmap`, formatted to fit with the rest of your document.

## Kernel Support for `mmap`

To support memory-mapped files, significant additions were made to the xv6 kernel, primarily revolving around the implementation of the `mmap` and `munmap` system calls, handling page faults on mapped regions, and ensuring correct VMA management during process creation (`fork`) and termination (`exit`).

### `mmap` and `munmap` Implementation

The `mmap` system call is responsible for creating a new memory mapping. Our implementation follows these steps:

1. It performs extensive argument validation, checking for valid protection flags, mapping flags, a valid file descriptor, and ensuring the requested length does not exceed the per-VMA `MMAPSIZE` limit.
2. It finds an available slot in the process's `p->vmas` array.
3. The virtual address for the mapping is deterministically assigned based on the VMA slot's index (`vma->start = MMAPBASE + vmaidx * MMAPSIZE`).
4. A new `vma` structure is populated with the specified length, protection, flags, and a pointer to the `struct file`.
5. Crucially, it increments the reference count of the associated file (`filedup`) to ensure the file structure is not deallocated even if the original file descriptor is closed.
6. No physical memory is allocated at this stage. The function simply returns the assigned virtual address to the user.

```c
void*
mmap(void *addr, uint length, int prot, int flags, int fd, int offset)
{
  struct proc *p;
  struct vma *vma;
  int vmaidx;

  p = myproc();

  // Check if the address is within the valid mmap range.
  if(addr != 0 && ((uint64)addr < MMAPBASE || (uint64)addr >= MMAPTOP)){
    printf("mmap: invalid address %p\n", addr);
    return (void*)-1;
  }

  // Check if the length is valid.
  if(length <= 0 || length > MMAPSIZE){
    printf("mmap: invalid length %d\n", length);
    return (void*)-1;
  }

  // Check if the file descriptor is valid.
  if(fd < 0 || fd >= NOFILE || p->ofile[fd] == 0){
    printf("mmap: invalid file descriptor %d\n", fd);
    return (void*)-1;
  }

  // Check if the protect flags are valid.
  if((prot & PROT_WRITE) && (flags & MAP_SHARED) && !(p->ofile[fd]->writable)){
    printf("mmap: file descriptor %d is not writable\n", fd);
    return (void*)-1;
  }

  // Check if the offset is valid.
  if(offset != 0){
    printf("mmap: only offset 0 is supported\n");
    return (void*)-1;
  }

  // Find a free VMA slot.
  for(vmaidx = 0; vmaidx < NVMA; vmaidx++){
    if(p->vmas[vmaidx].length == 0)
      break;
  }
  if(vmaidx == NVMA){
    printf("mmap: no free VMA slots\n");
    return (void*)-1;
  }

  vma = &p->vmas[vmaidx];
  vma->start = MMAPBASE + vmaidx * MMAPSIZE;
  vma->length = length;
  vma->prot = prot;
  vma->flags = flags;
  vma->file = p->ofile[fd];

  // Increment the reference count of the file and inode.
  filedup(vma->file);
  idup(vma->file->ip);

  return (void*)vma->start;
}
```

The `munmap` system call handles the removal of memory mappings.

1. It validates that the provided address and length fall within the bounds of an existing VMA.
2. It iterates through all pages within the requested unmapping range.
3. For each page, it checks if a valid Page Table Entry (PTE) exists. An invalid PTE is skipped, as it indicates the page was never accessed and thus requires no action.
4. If a valid, dirty (`PTE_D`) page is found within a `MAP_SHARED` mapping, its contents are written back to the corresponding location in the file using `filewrite`.
5. The kernel function `uvmunmap` is then called to invalidate the PTE and free the associated physical page.
6. The `vma`'s `start` and `length` fields are adjusted to reflect the unmapped portion.
7. If the entire VMA is unmapped (`vma->length` becomes 0), the associated file's reference count is decremented by calling `fileclose`.

```c
int
munmap(void *addr, uint length)
{
  struct proc *p;
  struct vma *vma;
  pte_t *pte;
  uint64 va, start;
  uint rwsize;
  int vmaidx, update_start;

  p = myproc();

  // Check if the address is within the valid mmap range.
  if((uint64)addr < MMAPBASE || (uint64)addr >= MMAPTOP){
    printf("munmap: invalid address %p\n", addr);
    return -1;
  }

  // Check if the length is valid.
  if(length <= 0 || length > MMAPSIZE){
    printf("munmap: invalid length %d\n", length);
    return -1;
  }

  start = va = PGROUNDDOWN((uint64)addr);
  length = PGROUNDUP(length);
  vmaidx = (va - MMAPBASE) / MMAPSIZE;
  vma = &p->vmas[vmaidx];

  // Check if the VMA exists and is valid.
  if(vma->length == 0){
    printf("munmap: no VMA found for address %p\n", addr);
    return -1;
  }

  // Check if the address range is within the VMA bounds.
  if(va < vma->start || va + length > vma->start + vma->length){
    printf("munmap: range %p to %p exceeds VMA bounds %p to %p\n", (void *)va,
           (void *)(va + length), (void *)vma->start, (void *)(vma->start + vma->length));
    return -1;
  }

  // Seek the file to the start of the VMA.
  vma->file->off = va - (MMAPBASE + vmaidx * MMAPSIZE);

  // Calculate the max remaining writable size of the file.
  rwsize = vma->file->ip->size - vma->file->off;

  // If the VMA starts at the beginning, we need to update the start.
  update_start = (start == vma->start);

  while(length > 0){
    pte = walk(p->pagetable, va, 0);
    // Unmap the page if it exists.
    if(pte && (*pte & PTE_V)){
      // If the page is dirty and the mapping is shared, write it back to the file.
      if((*pte & PTE_D) && (vma->flags & MAP_SHARED)){
        if(filewrite(vma->file, va, min(PGSIZE, rwsize)) < 0){
          printf("munmap: failed to write back dirty page at %p\n", (void*)va);
          return -1; 
        }
      }
      uvmunmap(p->pagetable, va, 1, 1);
    }
    length -= PGSIZE;
    rwsize -= PGSIZE;
    va += PGSIZE;

    // Update the VMA structure.
    vma->length -= PGSIZE;
    if(update_start){
      vma->start += PGSIZE;
    }
  }

  // Close the file if the VMA is now empty.
  if(vma->length == 0){
    // Decrement the reference count of the file and inode.
    fileclose(vma->file);
  }

  return 0;
}
```

A critical detail in the `munmap` implementation for `MAP_SHARED` regions is the correct invocation of `filewrite` to flush dirty pages. To prevent file corruption, two factors must be handled carefully: the file offset and the write size.

* **Offset Calculation**: Before writing, the file's internal offset (`vma->file->off`) must be set to the correct position. This is calculated by determining the dirty page's relative position within the VMA (`va - vma->start`) and adding it to the VMA's own starting offset within the file (`vma->offset`, which is 0 in this lab). This ensures the page is written back to the exact location from which it was read.

* **Write Size Limitation**: When writing the last page of a file mapping, the page may contain data that extends beyond the actual end-of-file. To avoid incorrectly extending the file's size with junk data from the end of the page, the write size is capped. We use `min(PGSIZE, remaining_file_size)` to ensure the `write` operation stops precisely at the file's end, preserving its original size.

### Page Fault Handling

The core of the lazy allocation strategy resides in the `usertrap` function, which now delegates page fault handling for the mmap region to a new function, `handlemmap`.

1\. **Fault Detection**: `handlemmap` checks for page faults and if the faulting address (`r_stval()`) falls within the dedicated `[MMAPBASE, MMAPTOP)` range.
2\. **VMA Validation**: `handlemmap` identifies the corresponding VMA for the faulting address. It validates that the address falls within the VMA's current bounds and that the access type (read or write, determined by `r_scause()`) is permitted by the VMA's protection flags (`vma->prot`).
3\. **Page Allocation and Population**: If the access is valid:

a. A new physical page is allocated using `kalloc()`.
b. The page is zeroed out to ensure that any part of the page that extends beyond the end of the file contains null bytes, as per POSIX specifications.
c. The `readi` function is called to read one page of content from the mapped file at the correct offset into the newly allocated physical page.

4\. **Page Table Mapping**: Finally, `mappages` is called to create a valid, user-accessible (`PTE_U`) PTE in the process's page table, mapping the faulting virtual page to the newly populated physical page with the correct read/write/execute permissions.

When `usertrap` returns, the faulting instruction is re-executed, and this time the memory access succeeds.

```c
int
handlemmap()
{
  struct proc *p;
  struct vma *vma;
  char *mem;
  uint64 addr, offset, scause;
  int vmaidx, perm;

  p = myproc();
  addr = r_stval();
  scause = r_scause();

  // Check if the scause indicates a page fault.
  if(scause != 0xd && scause != 0xf)
    return -1;

  // Check if the address is within the mmap range.
  if(addr < MMAPBASE || addr >= MMAPTOP){
    printf("handlemmap: not a valid mmap address 0x%lx\n", addr);
    return -1;
  }

  vmaidx = (addr - MMAPBASE) / MMAPSIZE;
  vma = &p->vmas[vmaidx];

  // Check if the VMA exists and is valid.
  if(vma->length == 0){
    printf("handlemmap: no VMA at index %d for address 0x%lx\n", vmaidx, addr);
    return -1;
  }

  // Check if the address is within the bounds of the VMA.
  if(addr < vma->start || addr >= vma->start + vma->length){
    printf("handlemmap: unmmapped address 0x%lx in VMA at index %d\n", addr, vmaidx);
    return -1;
  }

  // Check if the access is allowed based on the VMA's protection flags.
  if(scause == 0xf && (vma->prot & PROT_WRITE) == 0 && (vma->flags & MAP_SHARED)) {
    printf("handlemmap: write access to read-only mapping at address 0x%lx\n", addr);
    return -1;
  }

  // Allocate memory for the page.
  mem = (char*)kalloc();
  if(mem == 0){
    printf("handlemmap: memory allocation failed for address 0x%lx\n", addr);
    return -1;
  }

  // Remove the junk data from the allocated memory.
  memset(mem, 0, PGSIZE);

  offset = PGROUNDDOWN(addr - vma->start);
 
  // Read the data from the file associated with the VMA.
  ilock(vma->file->ip);
  if(readi(vma->file->ip, 0, (uint64)mem, offset, PGSIZE) < 0){
    printf("handlemmap: read failed for address 0x%lx, offset %lu\n", addr, offset);
    iunlock(vma->file->ip);
    kfree(mem);
    return -1;
  }
  iunlock(vma->file->ip);

  // Set the permissions for the page.
  perm = PTE_U;
  if(vma->prot & PROT_READ)
    perm |= PTE_R;
  if(vma->prot & PROT_WRITE)
    perm |= PTE_W;
  if(vma->prot & PROT_EXEC)
    perm |= PTE_X;

  // Map the page into the process's page table.
  if(mappages(p->pagetable, addr, PGSIZE, (uint64)mem, perm) < 0){
    printf("handlemmap: mmap failed for address 0x%lx\n", addr);
    kfree(mem);
    return -1;
  }

  return 0;
}
```

### VMA Handling in `fork()` and `exit()`

Process lifecycle events must also correctly manage VMAs to ensure proper inheritance and cleanup.

`fork()`: When a process is forked, the child must inherit the parent's memory mappings. Our implementation modifies `fork` to iterate through the parent's `vmas` array. For each active VMA, it:

* Copies the `vma` structure from the parent to the child's `vmas` array.
* Calls `filedup` on the `vma->file` pointer. This is critical as it increments the file's reference count, ensuring that the child's mapping remains valid even if the parent closes the file or exits.

```c
// copy mapped virtual memory areas.
for(i = 0; i < NVMA; i++){
  np->vmas[i] = p->vmas[i];
  if(p->vmas[i].length > 0){
    filedup(np->vmas[i].file);  // Increment file reference count.
    idup(np->vmas[i].file->ip); // Increment inode reference count.
  }
}
```

`exit()`: When a process exits, all its resources must be reclaimed. The `exit` function is modified to iterate through the process's `vmas` array. For each active VMA, it effectively calls `munmap` on the entire mapped region. This ensures that all `MAP_SHARED` dirty pages are written back to their files and all associated physical memory and page table entries are freed, preventing resource leaks.

```c
// Unmap all the mapped virtual memory areas.
for(vmaidx = 0; vmaidx < NVMA; vmaidx++){
  vma = &p->vmas[vmaidx];
  if(vma->length){
    start = vma->start;
    length = vma->length;
    if(munmap((void *)start, length) < 0){
      printf("exit: munmap failed for VMA at index %d\n", vmaidx);
    }
  }
}
```

## Relevant Files

* `kernel/proc.h`: Defines the `struct vma` and includes the fixed-size array of VMAs in `struct proc`.
* `kernel/proc.c`: Contains the `mmap` and `munmap` system call implementations, as well as modifications to `fork` and `exit`.
* `kernel/memlayout.h`: Defines the memory layout constants, including `MMAPBASE`, `MMAPTOP`, and `MMAPSIZE`.
* `kernel/trap.c`: Implements the `handlemmap` function to handle page faults for mapped regions.
* `kernel/sysproc.c`, `kernel/syscall.c`, `kernel/syscall.h`, `kernel/user.h`, `user/usys.pl`: Contains the new system call registration and user-space interface for `mmap` and `munmap`.

## Key Learning Outcomes

This lab was a deep dive into the implementation of a fundamental virtual memory feature found in modern operating systems. The key learning outcomes include:

* **Virtual Memory Area (VMA) Management**: Gained a concrete understanding of how a kernel tracks and manages distinct regions of a process's virtual address space. This involved designing and implementing the `struct vma` to represent memory mappings.
* **Lazy Allocation and Page Fault Handling**: Mastered the concept of lazy allocation (or demand paging). Instead of eagerly loading data, we implemented the logic to handle page faults as the primary mechanism for allocating physical memory and populating it from a file, which is central to `mmap`'s efficiency.
* **System Call Implementation**: Acquired hands-on experience in adding complex new system calls (`mmap`, `munmap`) to the kernel, including careful argument validation and state management.
* **File System and Memory Management Interaction**: Learned how the virtual memory system and the file system must interact. This included managing file reference counts (`filedup`), locking inodes (`ilock`), and using file I/O functions (`readi`, `filewrite`) within the page fault handler.
* **Process Lifecycle Integration**: Understood how advanced memory features must be integrated into the core process lifecycle, specifically by modifying `fork()` to correctly inherit memory mappings and `exit()` to ensure proper cleanup and resource deallocation.

## Challenges and Solutions

Implementing `mmap` and `munmap` presented several subtle but critical challenges, primarily related to correctness under the lazy allocation model and during process lifecycle events.

* **Handling Page Faults on Unmapped VMA Regions**: A significant challenge was correctly handling page faults. An early implementation of the page fault handler assumed a direct, static mapping between a faulting address and a VMA array index. This broke when `munmap` was called, as it would free the VMA but leave a stale mapping relationship.
* **Solution**: We implemented a robust `find_vma` function. Instead of calculating an index, this function iterates through the process's active VMAs to find the one that contains the faulting address. This ensures that faults on unmapped or previously unmapped regions are correctly identified as segmentation faults, rather than being handled incorrectly.
* **Partial Unmapping and File Write-back**: The `munmap` system call needed to correctly handle cases where only a portion of a VMA was unmapped. A particularly tricky aspect was writing dirty pages back to a file for `MAP_SHARED` mappings without corrupting the file.
* **Solution**: The `munmap` implementation was designed to carefully iterate through the page range being unmapped. For each dirty page, it calculates the precise offset within the file and, crucially, caps the write size using `min(PGSIZE, remaining_file_size)`. This prevents the `filewrite` operation from writing past the end of the file and incorrectly increasing its size.
* **Race Conditions in the Page Fault Handler**: In a concurrent setting, two threads could fault on the same page simultaneously. Without proper locking, this would lead to two separate physical pages being allocated and mapped to the same virtual address, causing a `remap` panic.
* **Solution**: We introduced a lock (e.g., `p->lock`) at the beginning of the page fault handling logic. After acquiring the lock, the handler re-checks if the page has been mapped by another thread in the intervening time. This double-checked locking pattern ensures that the page fault is handled exactly once.
* **Correct Resource Management in `fork()` and `exit()`**: Ensuring that file reference counts were managed correctly during `fork` and `exit` was critical to prevent resource leaks or premature deallocation. A simple VMA structure copy is insufficient.
* **Solution**: In `fork`, we explicitly call `filedup` for each inherited VMA to increment the `struct file` reference count. In `exit`, we ensure `munmap` is effectively called for all active VMAs, which in turn calls `fileclose`, correctly decrementing the reference count and releasing the file resources only when they are no longer in use by any process.

## Conclusion

This lab was a comprehensive exercise in implementing a sophisticated virtual memory feature. By adding `mmap` and `munmap`, we transformed the xv6 memory model from a simple heap-and-stack layout to a more flexible system capable of supporting advanced programming idioms. The core of the implementation was the VMA management system and the lazy-loading logic within the page fault handler. The challenges encountered, especially in handling concurrency, partial operations, and ensuring correct resource management across the entire process lifecycle, provided invaluable insight into the design and implementation of production-grade operating system kernels. The final result is a more powerful and efficient xv6 kernel that reflects a key aspect of modern OS design.
