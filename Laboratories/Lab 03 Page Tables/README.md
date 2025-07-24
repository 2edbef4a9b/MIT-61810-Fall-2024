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

# Lab 03 Page Tables

## Introduction

In this lab, we explore the xv6 virtual memory subsystem, focusing on performance optimization, debugging, and advanced feature implementation. The key tasks include inspecting page tables, optimizing system calls, and implementing superpage support.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout pgtbl
```

## Inspect Page Tables

Run `make qemu` and run the user program `pgtbltest`. The `print_pgtbl` functions prints out the page-table entries for the first 10 and last 10 pages of the `pgtbltest` process using the `pgpte` system call that we added to xv6 for this lab. The output looks as follows:

```bash
$ pgtbltest
print_pgtbl starting
va 0x0 pte 0x21FC885B pa 0x87F22000 perm 0x5B
va 0x1000 pte 0x21FC7C1B pa 0x87F1F000 perm 0x1B
va 0x2000 pte 0x21FC7817 pa 0x87F1E000 perm 0x17
va 0x3000 pte 0x21FC7407 pa 0x87F1D000 perm 0x7
va 0x4000 pte 0x21FC70D7 pa 0x87F1C000 perm 0xD7
va 0x5000 pte 0x0 pa 0x0 perm 0x0
va 0x6000 pte 0x0 pa 0x0 perm 0x0
va 0x7000 pte 0x0 pa 0x0 perm 0x0
va 0x8000 pte 0x0 pa 0x0 perm 0x0
va 0x9000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFF6000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFF7000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFF8000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFF9000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFFA000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFFB000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFFC000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFFD000 pte 0x0 pa 0x0 perm 0x0
va 0xFFFFE000 pte 0x21FD08C7 pa 0x87F42000 perm 0xC7
va 0xFFFFF000 pte 0x2000184B pa 0x80006000 perm 0x4B
print_pgtbl: OK
ugetpid_test starting
usertrap(): unexpected scause 0xd pid=4
            sepc=0x568 stval=0x3fffffd000
```

For every page table entry in the `print_pgtbl` output, explain what it logically contains and what its permission bits are. Figure 3.4 in the xv6 book might be helpful, although note that the figure might have a slightly different set of pages than process that's being inspected here. Note that xv6 doesn't place the virtual pages consecutively in physical memory.

**Page Table Entry (PTE) Format**:

* `va`: Virtual address (4KB page aligned)
* `pte`: Complete page table entry value
* `pa`: Physical address (translated from PTE)
* `perm`: Permission bits (lower 8 bits of PTE)

**Permission Bits Decoding**:

* 0x1 (1): Valid (V)
* 0x2 (2): Readable (R)
* 0x4 (4): Writable (W)
* 0x8 (8): Executable (X)
* 0x10 (16): User-accessible (U)
* 0x20 (32): Global mapping (G)
* 0x40 (64): Accessed (A)
* 0x80 (128): Dirty (D)

**Key Entries Analysis**:

1. `va 0x0` (User text):

* perm 0x5B (V+R+X+U+A+D): Executable code page
* Maps to physical `0x87F22000`

2. `va 0x1000` (User data):

* perm 0x1B (V+R+U+A): Readable data page

3. `va 0x2000` (User data):

* perm 0x17 (V+R+W+A): Writable data page

4. `va 0xFFFFE000` (Trampoline):

* perm 0xC7 (V+R+W+A+D): Kernel/user shared page
* Same physical page for all processes

5. `va 0xFFFFF000` (Trapframe):

* perm 0x4B (V+R+U+A): User-accessible kernel page
* Used for trap handling

**Zero Entries**:

* Pages with pte 0x0 are unmapped
* Seen in higher addresses (guard pages) and unused regions

**Important Notes**:

1. Physical pages are non-contiguous despite virtual continuity
2. User pages have U-bit set (0x10)
3. Kernel pages omit U-bit but remain user-accessible when needed (trapframe)
4. RWX permissions follow expected patterns for text/data sections

The output demonstrates how xv6:

* Separates user/kernel spaces
* Protects memory with proper permissions
* Shares certain pages between kernel/user
* Leaves guard pages unmapped for safety

## Speed Up System Calls

System calls are the fundamental interface between user applications and the kernel, but they carry a significant performance cost. Each call involves a hardware trap, saving and restoring the user context, switching to the kernel's context and stack, and finally returning to user space. For simple, frequently used, read-only system calls like `getpid()`, this overhead can vastly outweigh the actual work being done. To mitigate this bottleneck, we implemented a technique that uses a shared memory page to allow user processes to access certain kernel-provided data without initiating a full system call.

### The `USYSCALL` Page Mechanism

The core of this optimization is a per-process, read-only page mapped at a fixed virtual address, `USYSCALL`. This page acts as a low-overhead communication channel, allowing the kernel to expose simple, static information directly to the user process.

* **Shared Data Structure**: The `USYSCALL` page contains a `struct usyscall`, which is populated by the kernel when the process is created. For this lab, the structure holds the process's unique ID (`pid`).
* **Process Creation (`allocproc`)**: The `allocproc` function was modified to support this mechanism. Immediately after a new process is allocated and assigned a PID, the kernel:

1\. Allocates a new physical page using `kalloc()`.

2\. Populates this page with a `struct usyscall` containing the new process's PID.

3\. Maps this physical page into the new process's page table at the `USYSCALL` virtual address with read-only user permissions (`PTE_R | PTE_U`).

```c
static struct proc*
allocproc(void)
{
  // Existing code for process allocation...
  // Create a user syscall page for this process.
  struct usyscall *u = (struct usyscall *)kalloc();
  if (u == 0){
    proc_freepagetable(p->pagetable, p->sz);
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  u->pid = p->pid;

  // Allow user to access USYSCALL page.
  if(mappages(p->pagetable, USYSCALL, PGSIZE, (uint64)u, PTE_R | PTE_U) < 0) {
    kfree((void*)u);
    proc_freepagetable(p->pagetable, p->sz);
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Existing code to finalize process allocation...
}
```

* **Process Termination (`proc_freepagetable`)**: To prevent memory leaks, resource deallocation must mirror allocation. The `proc_freepagetable` function was updated to handle the cleanup of the `USYSCALL` page. When a process's page table is being destroyed, it now explicitly checks for a valid mapping at the `USYSCALL` address and, if found, unmaps the page and frees the underlying physical memory.

```c
// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);

  // Prevent double free of USYSCALL page.
  pte_t *pte = walk(pagetable, USYSCALL, 0);
  if(pte && (*pte & PTE_V)) {
    uvmunmap(pagetable, USYSCALL, 1, 1);
  }

  uvmfree(pagetable, sz);
}
```

With this design, a user-space call to `getpid()` no longer needs to trap into the kernel. It can be implemented as a simple memory read from the `USYSCALL` address, offering a substantial performance improvement for applications that frequently query their own PID.

Which other xv6 system call(s) could be made faster using this shared page? Explain how.

Many other read-only system calls could be made faster, e.g. `getpid`, `uptime`, `getppid`, `sysinfo`, the method is similar to the `ugetpid` implementation. The idea is to create a shared page in a read-only region and allow user to access it directly without trapping into the kernel.

## Print a Page Table

We implement a system call to print a process's page table with `vmprint`. This function iterates through the page table entries and prints out the virtual address, page table entry value, physical address for each entry. The implementation is similar to the `freewalk` function in `vm.c`, adapting recursive page table walking to print the relevant information.

```c
void
vmprint_recursive(pagetable_t pagetable, uint64 va, int level)
{
  pte_t pte;
  int i, j;

  for(i = 0; i < 512; i++) {
    pte = pagetable[i];
    if(pte & PTE_V) {
      for(j = 3; j > level; j--) {
        printf(" ..");
      }
      printf("%p: pte=%p, pa=%p\n", (void *)(va + (i << PXSHIFT(level))), (void *)pte,
             (void *)PTE2PA(pte));
    }
    if((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0) {
      uint64 child = PTE2PA(pte);
      vmprint_recursive((pagetable_t)child, va + (i << PXSHIFT(level)), level - 1);
    }
  }
}

void
vmprint(pagetable_t pagetable){
  if(pagetable == 0){
    printf("no page table\n");
    return;
  }
  printf("page table %p:\n", pagetable);
  vmprint_recursive(pagetable, 0, 2);
}
```

Example output of `vmprint`:

```bash
$ pgtbltest
print_kpgtbl starting
page table 0x0000000080124000:
 ..0x0000000000000000: pte=0x0000000020048001, pa=0x0000000080120000
 .. ..0x0000000000000000: pte=0x0000000020047c01, pa=0x000000008011f000
 .. .. ..0x0000000000000000: pte=0x000000002004845b, pa=0x0000000080121000
 .. .. ..0x0000000000001000: pte=0x000000002004781b, pa=0x000000008011e000
 .. .. ..0x0000000000002000: pte=0x00000000200474d7, pa=0x000000008011d000
 .. .. ..0x0000000000003000: pte=0x0000000020047007, pa=0x000000008011c000
 .. .. ..0x0000000000004000: pte=0x0000000020046cd7, pa=0x000000008011b000
 ..0xffffffffc0000000: pte=0x0000000020048c01, pa=0x0000000080123000
 .. ..0xffffffffffe00000: pte=0x0000000020048801, pa=0x0000000080122000
 .. .. ..0xffffffffffffe000: pte=0x00000000200504c7, pa=0x0000000080141000
 .. .. ..0xfffffffffffff000: pte=0x0000000020001c4b, pa=0x0000000080007000
```

Of course. Here is the report section for "Superpage Support," explaining the implementation details from the provided `kalloc.c` and `vm.c` files, following the established format.

## Superpage Support

Modern processors support mapping memory in larger chunks than the standard 4KB page size. On RISC-V, this is achieved by using a level 1 Page Table Entry (PTE) as a "leaf" to point directly to a 2MB physical page, known as a superpage. Using superpages significantly improves performance by reducing the number of entries in the Translation Lookaside Buffer (TLB) needed to map large regions of memory, which in turn reduces TLB misses and the costly page walks that follow. This section details the implementation of a 2MB superpage allocator and its integration into the xv6 virtual memory system.

### Superpage Allocation Mechanism

To support allocating both 4KB and 2MB pages, we implemented a simplified two-level buddy-style allocator. This system manages two distinct freelists and includes logic for splitting large pages into smaller ones and merging small pages back into large ones when possible.

**Data Structures (`kalloc.c`)**

The core of the physical allocator was extended with two key data structures:

1\. **Dual Freelist (`kmem`)**: The global `kmem` structure now maintains two separate linked lists: one for standard 4KB pages (`freelist`) and one for 2MB superpages (`superlist`).

```c
struct {
  struct spinlock lock;
  struct run *freelist;
  struct run *superlist;
} kmem;
```

2\. **Merge-Tracking Metadata (`freecount`)**: To know when a 2MB region's worth of 4KB pages are all free and can be merged, we introduced a metadata array. This array holds a counter for each potential 2MB superpage in physical memory.

```c
#define SUPERPGCOUNTS ((PHYSTOP - KERNBASE) / SUPERPGSIZE)

struct {
  uint count;
  struct spinlock lock;
} freecount[SUPERPGCOUNTS];
```

* `freecount[i].count` stores the number of free 4KB pages within the i-th 2MB physical region. A count of 512 indicates the region is fully free and ready for merging.
* A fine-grained spinlock protects each counter to allow for concurrent updates.

3\. **Allocation and Deallocation Flow**

* **Allocation (`kalloc`, `superalloc`)**:
  * `superalloc()`: Simply attempts to take a 2MB block from the `kmem.superlist`.
  * `kalloc()`: First tries to get a 4KB page from `kmem.freelist`. If the list is empty, it triggers the **split** mechanism by calling `ksplit()`.

* **Splitting (`ksplit`)**: If `kalloc` needs a 4KB page but finds `freelist` empty, `ksplit` takes a 2MB superpage from `superlist`, carves it into 512 individual 4KB pages, and adds all of them to the `freelist`.

* **Deallocation (`kfree`, `superfree`)**:
  * `superfree()`: Simply returns a 2MB block to the `superlist` and sets its corresponding `freecount` to 512.
  * `kfree()`: Returns a 4KB page to the `freelist`, increments its region's `freecount`, and then triggers the **merge** check by calling `kmerge()`.

* **Merging (`kmerge`)**: This function is the counterpart to `ksplit`.
    1. It first checks the `freecount` for the region containing the just-freed page. If the count has not reached 512, no merge is possible, and the function returns.
    2. If the count is 512, it acquires the global `kmem.lock` and proceeds to remove all 512 constituent 4KB pages from the `freelist`.
    3. Once all small pages are removed, the reconstituted 2MB superpage is added to the `superlist`.

```c
// In kmerge()
acquire(&freecount[USAGEIDX(pa)].lock);
if(freecount[USAGEIDX(pa)].count != 512){
  release(&freecount[USAGEIDX(pa)].lock);
  return; // Not ready to merge.
}
release(&freecount[USAGEIDX(pa)].lock);

acquire(&kmem.lock);
// Loop to remove all 512 pages from the freelist...
// Add the merged super page to the superlist...
release(&kmem.lock);
```

### Superpage Support in the Kernel

With the ability to allocate physical superpages, the virtual memory system (`vm.c`) was updated to map them correctly.

**Page Table Walking (`superwalk`)**

To create a superpage mapping, we need to access the level 1 PTE, not the level 0 PTE. A new function, `superwalk`, was created for this purpose. It is identical to the standard `walk` function but stops the page table traversal one level earlier.

```c
pte_t *
superwalk(pagetable_t pagetable, uint64 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  pte_t *pte = &pagetable[PX(2, va)];
  if(*pte & PTE_V) {
    pagetable = (pagetable_t)PTE2PA(*pte);
#ifdef LAB_PGTBL
    if(PTE_LEAF(*pte)) {
      return pte;
    }
#endif
  } else {
    if(!alloc || (pagetable = (pde_t *)kalloc()) == 0)
      return 0;
    memset(pagetable, 0, PGSIZE);
    *pte = PA2PTE(pagetable) | PTE_V;
  }
  return &pagetable[PX(1, va)];
}
```

**Mapping and Unmapping (`mapsuperpages`, `uvmunmap`)**

* A new function `mapsuperpages` was added, which uses `superwalk` to install a leaf PTE at level 1 of the page table, mapping a virtual address to a 2MB physical superpage.

```c
int
mapsuperpages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  if((va % SUPERPGSIZE) != 0)
    panic("mapsuperpages: va not aligned");

  if((size % SUPERPGSIZE) != 0)
    panic("mapsuperpages: size not aligned");

  if(size == 0)
    panic("mapsuperpages: size");

  a = va;
  last = va + size - SUPERPGSIZE;
  for(;;) {
    if((pte = superwalk(pagetable, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V)
      panic("mapsuperpages: remap");
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += SUPERPGSIZE;
    pa += SUPERPGSIZE;
  }
  return 0;
}
```

* `uvmunmap` was enhanced to detect if an address being unmapped is part of a superpage. It handles both full deallocation (calling `superfree`) and partial deallocation of a superpage (where the physical page is kept but the freed portion is zeroed out).

```c
void
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  uint64 a, pa;
  pte_t *pte, *spte;
  uint fillsz, offset;
  int sz;

  if((va % PGSIZE) != 0)
    panic("uvmunmap: not aligned");

  for(a = va; a < va + npages * PGSIZE; a += sz) {
    sz = PGSIZE;
    if((pte = walk(pagetable, a, 0)) == 0)
      panic("uvmunmap: walk");
    if((*pte & PTE_V) == 0) {
      printf("va=%p pte=%ld\n", (void *)a, *pte);
      panic("uvmunmap: not mapped");
    }
    if(PTE_FLAGS(*pte) == PTE_V)
      panic("uvmunmap: not a leaf");

    // Check if the PTE is mapped to a superpage.
    spte = superwalk(pagetable, SUPERPGROUNDDOWN(a), 0);
    if(spte && PTE_LEAF(*spte)) {
      sz = SUPERPGSIZE;

      // Check if partial free of superpage is required.
      if(va > SUPERPGROUNDDOWN(a)) {
        fillsz = min((SUPERPGROUNDUP(a) - va), npages * PGSIZE);
        offset = va - SUPERPGROUNDDOWN(a);
        pa = PTE2PA(*spte) + offset;
        memset((void *)pa, 1, fillsz);
        continue;
      }
    }

    if(do_free) {
      pa = PTE2PA(*pte);
      if(sz == PGSIZE) {
        kfree((void *)pa);
      } else if(sz == SUPERPGSIZE) {
        superfree((void *)pa);
      } else {
        panic("uvmunmap: bad sz");
      }
    }
    *pte = 0;
  }
}
```

**Opportunistic Superpage Allocation (`uvmalloc`)**

The user memory allocator, `uvmalloc`, was made "superpage-aware." It opportunistically uses superpages when conditions are favorable to improve performance.

When a process grows, `uvmalloc` checks if the current allocation address is 2MB-aligned and if the requested size is at least 2MB. If both conditions are true, it attempts to use a superpage.

```c
// In uvmalloc()
for(a = oldsz; a < newsz; a += sz) {
  sz = PGSIZE;
  // Check if the address is already mapped to a superpage.
  spte = superwalk(pagetable, SUPERPGROUNDDOWN(a), 0);
  if(spte && PTE_LEAF(*spte)) {
    sz = SUPERPGSIZE;
    continue;
  }

  // Check if we can map a superpage.
  if((a % SUPERPGSIZE) == 0 && (newsz - a) >= SUPERPGSIZE) {
    sz = SUPERPGSIZE;
  }

  if(sz == SUPERPGSIZE) {
    mem = superalloc();
    // ... map with mapsuperpages ...
  } else {
    mem = kalloc();
    // ... map with mappages ...
  }
}
```

**Fork Support (`uvmcopy`)**

Finally, `uvmcopy` was updated to correctly handle superpages when a process is forked. It detects if a region in the parent's address space is backed by a superpage and, if so, allocates a new superpage for the child and copies the full 2MB of content, ensuring memory layouts are preserved across `fork`.

```c
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte, *spte;
  uint64 pa, i;
  uint flags;
  char *mem;
  int szinc;

  for(i = 0; i < sz; i += szinc) {
    szinc = PGSIZE;
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");

    // Check if the PTE is mapped to a superpage.
    if((i % SUPERPGSIZE) == 0) {
      spte = superwalk(old, i, 0);
      if(spte == pte) {
        // This is a superpage mapping.
        szinc = SUPERPGSIZE;
      }
    }
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    if(szinc == PGSIZE) {
      if((mem = kalloc()) == 0)
        goto err;
      memmove(mem, (char *)pa, PGSIZE);
      if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0) {
        kfree(mem);
        goto err;
      }
    } else if(szinc == SUPERPGSIZE) {
      if((mem = superalloc()) == 0)
        goto err;
      memmove(mem, (char *)pa, SUPERPGSIZE);
      if(mapsuperpages(new, i, SUPERPGSIZE, (uint64)mem, flags) != 0) {
        superfree(mem);
        goto err;
      }
    } else {
      panic("uvmcopy: bad szinc");
    }
  }
  return 0;

err:
  uvmunmap(new, 0, i / PGSIZE, 1);
  return -1;
}
```

## Relevant Files

* `kernel/kalloc.c`: Contains the implementation of the buddy-style allocator, including the logic for splitting and merging pages.
* `kernel/vm.c`: Implements the `vmprint` function and superpage support, including the `superwalk`, `mapsuperpages`, `uvmunmap`, etc.
* `kernel/proc.c`: Modified to allocate and manage the `USYSCALL` page for each process.

## Key Learning Outcomes

This lab provided a comprehensive, hands-on exploration of the xv6 virtual memory subsystem, focusing on performance optimization, debugging, and advanced feature implementation. The key takeaways span from the user-kernel interface down to the physical memory allocator.

* **Optimizing the User-Kernel Boundary**: Mastered the use of shared memory (`USYSCALL` page) as a high-performance mechanism to bypass costly system call traps for simple, read-only data. This demonstrated a powerful pattern for API design that leverages the MMU to reduce software overhead.
* **Visualizing Complex Kernel Structures**: Learned the importance of kernel introspection by implementing `vmprint`. This tool provided invaluable insight into the hierarchical nature of page tables, making it possible to debug complex memory mapping issues by directly visualizing the state of the MMU for a given process. It solidified the understanding of multi-level address translation.
* **Advanced Physical Memory Management**: Gained practical experience designing and implementing a two-level buddy-style allocator. This involved creating data structures to manage memory chunks of different sizes (4KB and 2MB) and implementing the core logic for **splitting** large pages on-demand and **merging** small pages opportunistically.
* **Hardware Feature Integration (Superpages)**: Learned to harness a key CPU performance feature by integrating superpage support throughout the kernel. This was not just an allocator change but a holistic effort that required modifying page table walking (`superwalk`), memory mapping (`mapsuperpages`), and all major user memory VFS operations (`uvmalloc`, `uvmdealloc`, `uvmcopy`).
* **Understanding Performance Trade-offs**: This lab highlighted the constant trade-offs in kernel design. The superpage allocator is significantly more complex than the original, but offers substantial performance gains by reducing TLB misses. The `USYSCALL` page adds setup complexity but makes `getpid` almost free. These exercises demonstrated how to reason about when and why such trade-offs are beneficial.

## Challenges and Solutions

Implementing these wide-ranging features involved overcoming several classic kernel development challenges:

* **Symmetrical Resource Management**: A core challenge in adding the `USYSCALL` page was preventing memory leaks. Any resource allocated during process creation must be freed upon termination.
* **Solution**: This was solved by ensuring symmetrical design. The `allocproc` function was modified to allocate and map the `USYSCALL` page, and its counterpart, `proc_freepagetable`, was updated to unmap and free that same page, guaranteeing no resources were leaked.
* **Concurrency and Race Conditions in Merging**: The `kmerge` function presented a significant concurrency challenge. If two CPUs free the final pages of a 2MB block simultaneously, they could both see the `freecount` reach 512 and attempt to perform the merge, leading to data corruption and system crashes.
* **Solution**: This was addressed by using a combination of fine-grained and coarse-grained locks. A per-superpage-region lock (`freecount.lock`) protects the counter check, while the global `kmem.lock` serializes the actual, complex operation of removing pages from the freelist and adding the new superpage to its list. While this works, it reveals a deeper challenge in designing highly concurrent allocators.
* **Inefficient Merge Operation**: The implemented `kmerge` logic, upon deciding to merge, must scan the entire 4KB freelist to find and remove the 512 pages belonging to the target superpage. This is an O(N) operation that can become a performance bottleneck inside `kfree`.
* **Solution**: For this lab, the simple (but slow) implementation was sufficient. A more advanced production-grade buddy system would solve this by maintaining separate freelists for each "order" of block size. This would allow the merge operation to be O(1), as the two buddies would be the only occupants of their order's freelist before being moved to the next-higher order.
* **Correctly Handling Hierarchical Page Tables**: Implementing `vmprint` and superpage support both required a deep and precise understanding of the RISC-V multi-level page table structure.
* **Solution**: For `vmprint`, a recursive traversal function was the natural solution to print the tree structure. For superpages, a new `superwalk` function was created to stop the walk at level 1, allowing the kernel to correctly install a leaf PTE at the appropriate level to map a 2MB page.

## Conclusion

This lab was a transformative exercise in evolving the xv6 kernel's memory subsystem from a simple, functional implementation into a more performant, debuggable, and feature-rich system. By tackling optimizations at multiple layers—from the system call interface down to the physical page allocator—we achieved significant performance gains and added capabilities that mirror those in modern operating systems.

The implementation of superpage support with a dynamic splitting and merging allocator was the cornerstone of this effort, providing a deep dive into the complexities of memory management algorithms and their interaction with CPU hardware. Supplemented by the practical `USYSCALL` optimization and the invaluable `vmprint` debugging utility, this lab provided a holistic view of how software and hardware co-operate to manage memory efficiently. The final result is a more scalable and sophisticated kernel, better equipped to handle the demands of memory-intensive applications in a multi-core world.
