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

# Lab 05 Copy-on-Write Fork

## Introduction

In this lab, we implemented Copy-on-Write (COW) fork in the xv6 operating system. COW is an optimization technique that allows multiple processes to share the same physical memory pages until one of them attempts to modify a page. This reduces memory usage and improves performance by avoiding unnecessary copying of pages during the `fork` operation.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout cow
```

## Reference Counting

To support copy-on-write (COW) fork, we need to implement reference counting for pages. This allows us to track how many processes share a page and only free the page when no processes are using it. A `kref` structure is added to `kernel/kalloc.c` to manage the reference count for each page.

```c
struct {
  struct spinlock lock;
  int count[(PHYSTOP - KERNBASE) / PGSIZE];
} kref;
```

The index of a page in the `kref.count` array is calculated as `(pa - KERNBASE) / PGSIZE`, where `pa` is the physical address of the page. The `kref` structure is protected by a spinlock to ensure thread safety when accessing or modifying the reference counts.

We also need to modify the `kalloc` and `kfree` functions to maintain the reference count, together with a newly added `krefinc` function to increment the reference count when a page is shared.

1\. `kalloc` function, initializing the reference count to 1 when a page is allocated

```c
void *
kalloc(void)
{
  // ...existing code...
  if(r){
    // ...existing code...
    acquire(&kref.lock);
    if (kref.count[index] != 0) {
      release(&kref.lock);
      panic("kalloc: page already allocated");
    }
    kref.count[index] = 1; // Set count to 1 on allocation
    release(&kref.lock);
  }
  // ...existing code...
}
```

2\. `kfree` function, which decrements the reference count and only frees the page if the count reaches zero. If the count is greater than zero, it does not free the page, allowing other processes to continue using it.

```c
void
kfree(void *pa)
{
  // ...existing code...
  acquire(&kref.lock);
  kref.count[index]--;
  if (kref.count[index] > 0) {
    release(&kref.lock);
    return; // Don't free if there are still references.
  }
  if (kref.count[index] < 0) {
    release(&kref.lock);
    panic("kfree: negative reference count");
  }
  release(&kref.lock);

  // Fill with junk and add to freelist only if count is 0
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}
```

3\. `krefinc` function, which increments the reference count for a given physical address. This is used when a page is shared between processes.

```c
void
krefinc(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("krefinc");

  int index = ((uint64)pa - KERNBASE) / PGSIZE;
  acquire(&kref.lock);
  kref.count[index]++;
  release(&kref.lock);
}
```

A helper function `krefget` is also added to retrieve the current reference count for a given physical address.

```c
int
krefget(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("krefget");

  int index = ((uint64)pa - KERNBASE) / PGSIZE;
  acquire(&kref.lock);
  int count = kref.count[index];
  release(&kref.lock);
  return count;
}
```

**Relevant Files:**

* `kernel/kalloc.c`: Contains the implementation of `kalloc`, `kfree`, `krefinc`, and `krefget`.
* `kernel/defs.h`: Contains the declaration of `krefinc` and `krefget`.

## COW Fork

The `uvmcopy` function is used to create a copy of the user memory space for a new process. To implement COW, several modifications are made:

* Share Pages: Instead of allocating a new physical page (`kalloc`) and copying the content (`memmove`) for every page in the parent's address space, `uvmcopy` now maps the same physical page (`pa`) from the parent's page table (`old`) into the child's page table (`new`).
* Modify PTEs: For pages that were originally writable (`*pte & PTE_W`), the write permission (`PTE_W`) is removed from the PTE in both the parent's and the child's page tables. A new flag, `PTE_COW`, is set in both PTEs to mark them as Copy-on-Write pages. Pages that were not originally writable (e.g., text segment) remain read-only and shared without the `PTE_COW` flag.
* Increment Reference Count: For each physical page that is shared (i.e., mapped into the child's page table), `krefinc` is called to increment its reference count.

```c
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  uint64 pa, i;
  uint flags;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");
    pa = PTE2PA(*pte);

    // Copy on write.
    if (*pte & PTE_W) {
      *pte &= ~PTE_W;
      *pte |= PTE_COW;
    }
    krefinc((void *)pa);
    flags = PTE_FLAGS(*pte);

    // Copy the page table entry to the new page table.
    if(mappages(new, i, PGSIZE, pa, flags) != 0){
      uvmunmap(new, 0, i / PGSIZE, 1);
      return -1;
    }
  }
  return 0;
}
```

The `PTE_COW` flag is defined in `kernel/riscv.h` to indicate that a page is marked for Copy-on-Write, we use the reserved bit in the PTE flags.

```c
#define PTE_COW (1L << 8) // copy-on-write flag in reserved bits
```

**Relevant Files:**

* `kernel/vm.c`: Contains the modified `uvmcopy` function.
* `kernel/riscv.h`: Contains the definition of `PTE_COW`.

## COW Page Fault Handling

When a process attempts to write to a page that is marked as Copy-on-Write, a page fault occurs. We handle this fault in `handle_cow` function when a store page fault occurs:

* Check if the faulting address is valid and if the page is marked as `PTE_COW`, if not, just return and let the kernel handle it as a normal page fault.
* If the page is marked as `PTE_COW` and the reference count is 1, we can simply remove the `PTE_COW` flag and set the `PTE_W` flag to allow writing.
* If the reference count is greater than 1, we need to:
  * Allocate a new physical page using `kalloc`.
  * Copy the content from the original page to the new page.
  * Update the PTE in the page table to point to the new page, setting the `PTE_W` flag and removing the `PTE_COW` flag.
  * Decrement the reference count of the original page by calling `kfree` on it.

```c
int
handle_cow()
{
  int ref_count;
  uint64 pa_old;
  pte_t *pte;

  struct proc *p = myproc();
  uint64 stval = r_stval();

  if(stval >= MAXVA) {
    // Invalid address.
    return -1;
  }

  pte = walk(p->pagetable, stval, 0);

  if(!pte || (*pte & PTE_COW) == 0) {
    // Not a COW page fault.
    printf("handle_cow: not a COW page fault at 0x%lx\n", stval);
    return -1;
  }

  pa_old = PTE2PA(*pte);
  ref_count = krefget((void *)pa_old);

  if(ref_count < 1) {
    panic("handle_cow: invalid ref count for COW page");
  }
  if(ref_count == 1) {
    // Only one reference, so we can just write to the page.
    *pte &= ~PTE_COW; // Clear the COW flag.
    *pte |= PTE_W;    // Set write permission.
    return 0;
  }

  // Allocate a new page.
  void *new_page = kalloc();
  if(!new_page){
    // Out of memory.
    printf("handle_cow: out of memory for COW page at 0x%lx\n", stval);
    return -1;
  }

  // Copy the contents of the old page to the new page.
  if(memmove(new_page, (void *)pa_old, PGSIZE) == 0){
    // Failed to copy the page.
    printf("handle_cow: failed to copy COW page at 0x%lx\n", stval);
    kfree(new_page);
    return -1;
  }

  // Update the page table entry to point to the new page.
  pte_t new_pte = PA2PTE(new_page) | PTE_FLAGS(*pte);
  new_pte &= ~PTE_COW; // Clear the COW flag.
  new_pte |= PTE_W;    // Set write permission.
  *pte = new_pte;

  // Call kfree to decrement the reference count of the old page.
  kfree((void *)pa_old);

  return 0;
}
```

The `copyout` function also needs modification to handle COW pages because it may write to a page that is marked as COW. Before writing data to a user page, it checks if the corresponding PTE has the `PTE_COW` flag set. If it does, it performs the same COW break logic as in `handle_cow`: check the reference count, allocate a new page if necessary, copy the data, update the PTE, and decrement the reference count of the old page.

```c
int
copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  uint64 n, va0, pa0;
  pte_t *pte;
  int ref_count;

  while(len > 0){
    va0 = PGROUNDDOWN(dstva);
    if(va0 >= MAXVA)
      return -1;
    pte = walk(pagetable, va0, 0);
    if(pte == 0 || (*pte & PTE_V) == 0 || ((*pte & PTE_U) == 0) ||
      (((*pte & PTE_W) == 0) && ((*pte & PTE_COW) == 0)))
      return -1;

    if (*pte & PTE_COW) {
      pa0 = PTE2PA(*pte);
      ref_count = krefget((void*)pa0);

      if(ref_count < 1){
        panic("copyout: invalid ref count for COW page");
      }
      if (ref_count == 1){
        // Only one reference, so we can just write to the page.
        *pte &= ~PTE_COW; // Clear the COW flag.
        *pte |= PTE_W;    // Set write permission.
      } else {
        // Allocate a new page.
        void *new_page = kalloc();
        if(!new_page){
          // Out of memory.
          panic("copyout: out of memory for COW page");
        }

        // Copy the contents of the old page to the new page.
        if (memmove(new_page, (void*)pa0, PGSIZE) == 0) {
          // Failed to copy the page.
          kfree(new_page);
          return -1;
        }

        // Update the page table entry to point to the new page.
        pte_t new_pte = PA2PTE(new_page) | PTE_FLAGS(*pte);
        new_pte &= ~PTE_COW; // Clear the COW flag.
        new_pte |= PTE_W; // Set write permission.
        *pte = new_pte;

        // Call kfree to decrement the reference count of the old page.
        kfree((void*)pa0);
      }
    }
    pa0 = PTE2PA(*pte);
    n = PGSIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)(pa0 + (dstva - va0)), src, n);

    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }
  return 0;
}
```

**Relevant Files:**

* `kernel/vm.c`: Contains the modified `copyout` function.
* `kernel/trap.c`: Contains the `handle_cow` function to handle COW page faults.
* `kernel/defs.h`: Contains the declaration of `handle_cow`.

### Key Learning Outcomes

This lab provided a deep dive into advanced virtual memory management techniques, specifically focusing on the Copy-on-Write (COW) fork mechanism. Key learning outcomes include:

* **Page Tables and PTE Flags:** Learning how to manipulate page table entries (PTEs) and utilize PTE flags (like `PTE_V`, `PTE_W`, `PTE_U`, and the newly introduced `PTE_COW`) to control memory access permissions and implement COW behavior.
* **Reference Counting:** Implementing and managing a physical page reference counting system (`kref`) to track shared pages and ensure they are only freed when no longer referenced by any process. Understanding the importance of synchronization (using spinlocks) for shared data structures like the reference count array.
* **Trap Handling for Page Faults:** Modifying the trap handler (`usertrap`) to specifically identify and handle page faults (`scause=0xd` and `0xf`), and implementing a dedicated function (`handle_cow`) to process COW page faults.
* **Copy-on-Write Logic:** Understanding the core COW principle: sharing pages initially and only copying them when a write attempt occurs on a shared page. Implementing the COW break logic in `handle_cow` and `copyout`.
* **Interplay of OS Components:** Observing how different parts of the OS (process management, memory allocation, trap handling, system calls like `fork` and `exec`) interact and need to be coordinated to support COW.

### Challenges and Solutions

Implementing the COW fork mechanism presented several challenges:

* **Correct Reference Counting:** Ensuring that reference counts are incremented and decremented correctly in all relevant scenarios (`kalloc`, `kfree`, `uvmcopy`, `uvmunmap`, `handle_cow`, `copyout`). A common pitfall is double-freeing pages or having negative reference counts due to incorrect decrementing logic, especially when pages are shared. The solution involved carefully adding `krefinc` and modifying `kfree` to only return pages to the freelist when the count reaches zero.
* **Modifying `uvmcopy`:** The primary challenge was changing `uvmcopy` from a simple page-copying function to one that shares physical pages and correctly modifies PTEs in both parent and child page tables (clearing `PTE_W`, setting `PTE_COW`).
* **Handling COW Page Faults:** Implementing `handle_cow` required correctly identifying COW page faults, checking reference counts, allocating new pages, copying data, updating the PTE, and decrementing the old page's reference count. Edge cases like out-of-memory during allocation needed to be handled gracefully.
* **Integrating with `copyout`:** Recognizing that `copyout` also performs writes to user memory and needs to trigger the COW break mechanism was important. Modifying `copyout` to check for `PTE_COW` and perform the necessary copy logic was required.
* **Debugging Page Table Issues:** Debugging page table manipulation and trap handling often involves examining register values (`scause`, `stval`, `sepc`) and page table entries, which can be complex. Using print statements and GDB was essential for tracing execution flow and identifying incorrect PTE states or memory accesses.

### Conclusion

Lab 05 provided valuable practical experience in implementing a fundamental operating system optimization: Copy-on-Write fork. By deferring the copying of memory pages until they are actually modified, COW significantly improves the performance of `fork` by reducing memory consumption and CPU overhead. This lab reinforced our understanding of virtual memory concepts, page table management, and the critical role of trap handling in responding to memory access events. Implementing the reference counting and integrating the COW logic into the existing xv6 memory management and trap handling code demonstrated the interconnectedness of different OS components and the challenges involved in building efficient and correct memory sharing mechanisms.
