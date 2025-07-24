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

# Lab 02 System Calls

## Introduction

In this lab, we use GDB to debug the xv6 kernel and user programs, implement a system call `trace`, and explore a security vulnerability in the xv6 kernel. We will also implement a user program that exploits this vulnerability to retrieve a secret password.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout syscall
```

## Using GDB

### Debugging xv6

To debug xv6, we can use GDB with the following command:

```bash
make qemu-gdb
```

Run `riscv64-linux-gnu-gdb`, and connect to the GDB server:

```bash
riscv64-linux-gnu-gdb
(gdb) file kernel/kernel
(gdb) target remote localhost:26000
```

![Image 01 Qemu GDB](../../Images/Lab%2002%20System%20Calls/Image%2001%20Qemu%20GDB.png)

Looking at the backtrace output, which function called `syscall` ?

> `usertrap`

![Image 02 Backtrace](../../Images/Lab%2002%20System%20Calls/Image%2002%20Backtrace.png)

What is the value of `p->trapframe->a7` and what does that value represent? (Hint: look `user/initcode.S`, the first user program xv6 starts.)

> 7, `SYS_exit`, which is the system call number for `exit`.

![Image 03 Trapframe](../../Images/Lab%2002%20System%20Calls/Image%2003%20Trapframe.png)

What was the previous mode that the CPU was in?

```bash
(gdb) p /x $sstatus
$4 = 0x200000022
```

`0x200000022 = 0b 0010 0000 0000 0000 0000 0000 0010 0010`, where the eighth bit (SPP) is set to 0, The SPP (Supervisor Previous Privilege) bit tells us the mode before the current trap occurred:

* SPP = 0 -> User mode
* SPP = 1 -> Supervisor mode

> User mode

### Debugging Kernel Panic

```diff
In kernel/syscall.c
@@ -134,7 +134,8 @@ syscall(void)
   int num;
   struct proc *p = myproc();
 
-  num = p->trapframe->a7;
+  // num = p->trapframe->a7;
+  num = * (int *) 0;
   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
     // Use num to lookup the system call function for num, call it,
     // and store its return value in p->trapframe->a0
```

After modifying the code and run `make qemu`, we see output like:

```bash
hart 2 starting
hart 1 starting
scause=0xd sepc=0x80002a12 stval=0x0
panic: kerneltrap
```

where `sepc=0x80002a12` is the Supervisor Exception Program Counter, the address of the instruction that caused the exception.

![Image 04 Kernel Panic](../../Images/Lab%2002%20System%20Calls/Image%2004%20Kernel%20Panic.png)

Corresponding assembly codes in `kernel/kernel.asm`:

```assembly
  struct proc *p = myproc();
    80002a0c: a9dfe0ef           jal 800014a8 <myproc>
    80002a10: 84aa                 mv s1,a0

  num = * (int *) 0;
    80002a12: 00002683           lw a3,0(zero) # 0 <_entry-0x80000000>
```

Write down the assembly instruction the kernel is panicing at. Which register corresponds to the variable `num`?

> `lw a3,0(zero)`, where `a3` corresponds to the variable `num`.

Why does the kernel crash? Hint: look at figure 3-3 in the text; is address 0 mapped in the kernel address space? Is that confirmed by the value in `scause` above? (See description of `scause` in RISC-V privileged instructions)

The `scause` value is `0xd` (decimal 13), which indicates a "Load Page Fault" exception according to the RISC-V privileged specification.

![Image 05 Memory Map](../../Images/Lab%2002%20System%20Calls/Image%2005%20Memory%20Map.png)

Address 0 is not mapped in the kernel address space, which is why the kernel panics when trying to load from it. The `scause` value confirms that a page fault occurred, indicating that the kernel attempted to access an invalid memory address.

> The kernel crashes due to a page fault.

What is the name of the process that was running when the kernel paniced? What is its process id (pid)?

```bash
(gdb) p p->name
$2 = "initcode\000\000\000\000\000\000\000"
(gdb) p p->pid
$3 = 1
```

> `initcode`, 1

![Image 06 Process Info](../../Images/Lab%2002%20System%20Calls/Image%2006%20Process%20Info.png)

## System Call Tracing

In this part of the lab, we implemented a system call tracing feature for the xv6 operating system. The goal was to allow users and developers to observe which system calls are invoked by a process, along with their return values, for debugging and educational purposes.

**Source Code Location:**

* Kernel changes: `/kernel/syscall.c`, `/kernel/sysproc.c`, `/kernel/proc.h`
* User program: `/user/trace.c`

**Key Features:**

* Added a new system call `trace` that enables or disables tracing for a process.
* Modified the kernel’s system call dispatcher to print the system call name, process id, and return value when tracing is enabled.
* Used a bitmask to allow selective tracing of specific system calls.
* Provided a user-level program `trace` to set the tracing mask for a process and its children.

**Usage:**

```bash
$ trace 32 grep hello README
3: syscall read -> 1023
3: syscall read -> 966
3: syscall read -> 70
3: syscall read -> 0
$
$ trace 2147483647 grep hello README
4: syscall trace -> 0
4: syscall exec -> 3
4: syscall open -> 3
4: syscall read -> 1023
4: syscall read -> 966
4: syscall read -> 70
4: syscall read -> 0
4: syscall close -> 0
$
$ grep hello README
$
$ trace 2 usertests forkforkfork
usertests starting
test forkforkfork: 407: syscall fork -> 408
408: syscall fork -> 409
409: syscall fork -> 410
410: syscall fork -> 411
409: syscall fork -> 412
410: syscall fork -> 413
409: syscall fork -> 414
411: syscall fork -> 415
...
$
```

## Attack Xv6

For this lab, a deliberate bug is introduced: when a user process requests new memory pages, the kernel does not clear the contents of those pages before handing them out. As a result, newly allocated memory may contain leftover data from previous processes.

The `secret` program in `user/secret.c` allocates a large chunk of heap memory (32 pages) using `sbrk`, then advances the pointer by 9 pages. It then writes a secret message to the start of the 10th page and appends a 8-byte password to it at offset 32 bytes.

```c
char *end = sbrk(PGSIZE * 32);
end = end + 9 * PGSIZE;
strcpy(end, "my very very very secret pw is:   ");
strcpy(end + 32, argv[1]);
```

Since the kernel does not clear the memory contents before reallocation, this creates an important security vulnerability. When the `secret` program exits, its allocated pages are freed and returned to the kernel's memory pool, The page contents remain intact (including both the pattern string and secret password).

However the first 8 bytes in the freed pages will be overwritten with a freelist pointer when added to the free list. This behavior can be found in `kernel/kalloc.c`:

```c
void
kfree(void *pa)
{
  struct run *r;

  // Existing codes ...

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;  // Add the freed page to the free list.
  release(&kmem.lock);
}
```

The first 8 bytes of freed pages are overwritten with a freelist pointer due to how the memory management structures are designed in xv6. In `kernel/kalloc.c`, the kernel defines a `struct run` that serves as the fundamental unit for tracking free memory pages. The structure is defined as:

```c
struct run {
  struct run *next;
};
```

This minimal structure contains just a single `next` pointer that points to the next free page in the list. On a 64-bit system architecture (which xv6 is configured for by default), each pointer occupies exactly 8 bytes of memory.

The assignment `r->next = kmem.freelist` writes the current head of the free list into the `next` field of the newly freed page, which occupies the first 8 bytes of that physical page.

Based on the above information, we can exploit this vulnerability by the following steps:

1\. Running the `secret` user program and write the pattern and the password to a page in memory.

2\. Running the `attack` user program immediately after `secret` exits.

3\. In `attack`, we allocate a large chunk of heap memory using `sbrk`, and iterate through the pages to find the page that contains the pattern string with:

```c
if (memcmp(ptr + 8, pattern + 8, 24) == 0)
```

where `ptr` is the page pointer, `pattern` is the pattern string, and `8` is the offset to skip the freelist pointer.

4\. Once the page is found, we can extract the password at `ptr + 32`, and write to fd 2.

```c
char *secret_addr = ptr + 32;
write(2, secret_addr, 8);
exit(0);
```

`user/secret.c` copies the secret bytes to memory whose address is 32 bytes after the start of a page. Change the 32 to 0 and you should see that your attack doesn't work anymore; why not?

> If we change the offset from 32 to 0, the password will be written at the start of the page, which will be overwritten by the freelist pointer when the page is freed.

## Key Learning Outcomes

* **System Call Implementation:** Gained hands-on experience in adding new system calls to the xv6 kernel and understanding the system call dispatch mechanism.
* **Kernel and User Debugging:** Learned to use GDB to debug both user programs and kernel code, including analyzing kernel panics and process state.
* **Process Isolation and Security:** Explored how improper memory management in the kernel can lead to security vulnerabilities, and understood the importance of clearing memory before reallocation.
* **Bitmask and Tracing Techniques:** Practiced using bitmasks for selective system call tracing and developed tools to observe kernel-user interactions.
* **Memory Management Internals:** Investigated how xv6 manages physical and virtual memory, including the use of free lists and the implications for security.

## Challenges and Solutions

* **Understanding Kernel Data Structures:** It was challenging to trace how memory pages are managed and reused in xv6. Careful reading of the kernel source and use of GDB helped clarify the flow of memory allocation and freeing.
* **Debugging Across User and Kernel Space:** Debugging issues that span user and kernel space required learning how to interpret trapframes, process states, and memory mappings using GDB.
* **Correct Bitmask Handling:** Ensuring that the correct system call numbers were mapped to the correct bits in the tracing mask required attention to detail and testing with various system calls.
* **Exploiting the Vulnerability Reliably:** Crafting the attack to reliably recover the secret required understanding the freelist pointer overwrite and adjusting the scanning logic to skip the first 8 bytes of each page.

## Conclusion

Lab 02 provided practical experience with the implementation and debugging of system calls in xv6, as well as a deeper understanding of kernel memory management and process isolation. By tracing system calls and exploiting a deliberate vulnerability, we saw firsthand how small bugs in the kernel can have significant security consequences. The lab reinforced the importance of careful kernel programming and provided valuable skills in debugging, kernel development, and
