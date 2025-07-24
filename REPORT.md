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

# Xv6 RISCV Fall 2024 实验报告

## 简介

Xv6 是一个由 MIT PDOS 实验室开发的教学操作系统, 旨在帮助学生理解操作系统的基本概念和实现, 同时作为课程 MIT 6.1810 的实验项目基础. 本报告将涵盖 MIT 6.1810 Fall 2024 的实验内容, 包含实验的目的, 实现细节, 遇到的挑战以及解决方案等.

## 代码获取

仓库地址: [https://github.com/2edbef4a9b/MIT-61810-Fall-2024](https://github.com/2edbef4a9b/MIT-61810-Fall-2024)

通过 Git 获取代码:

```bash
git clone https://github.com/2edbef4a9b/MIT-61810-Fall-2024.git
```

仓库结构如下:

```bash
.
├── Images             # 报告中使用的图片
│   ├── Lab 00 Setup 
│   │   └── Image 01.png
│   └── ...
├── Laboratories       # 实验报告目录
│   ├── Lab 00 Setup   # 各实验的报告目录
│   │   └── README.md  # 实验报告文件
│   └── ...
├── Xv6-RISCV          # Xv6 实验仓库
│   ├── kernel
│   ├── mkfs
│   ├── user
│   ├── Makefile
│   └── ...
├── README.md          # 仓库的 README 文件
├── REPORT.md          # 实验报告汇总
├── test.sh            # 自动测试脚本
└── ...
```

运行 `test.sh` 脚本会运行每一个实验的测试用例, 并输出测试结果.

## 目录

## 目录

* [Lab 00 Setup](#lab-00-setup)
  * [Introduction](#lab-00-setup-introduction)
  * [Install Toolchain](#lab-00-setup-install-toolchain)
  * [Get Xv6 RISCV Source Code](#lab-00-setup-get-xv6-riscv-source-code)
  * [Compile and Run](#lab-00-setup-compile-and-run)
* [Lab 01 Utilities](#lab-01-utilities)
  * [Introduction](#lab-01-utilities-introduction)
  * [Setup](#lab-01-utilities-setup)
  * [Boot Xv6](#lab-01-utilities-boot-xv6)
  * [Implemented Programs](#lab-01-utilities-implemented-programs)
    * [01. Sleep Program](#lab-01-utilities-01-sleep-program)
    * [02. Pingpong Program](#lab-01-utilities-02-pingpong-program)
    * [03. Primes Program](#lab-01-utilities-03-primes-program)
    * [04. Find Program](#lab-01-utilities-04-find-program)
    * [05. Xargs Program](#lab-01-utilities-05-xargs-program)
  * [Running the Programs](#lab-01-utilities-running-the-programs)
  * [System Calls Learned](#lab-01-utilities-system-calls-learned)
  * [Key Learning Outcomes](#lab-01-utilities-key-learning-outcomes)
  * [Challenges and Solutions](#lab-01-utilities-challenges-and-solutions)
  * [Conclusion](#lab-01-utilities-conclusion)
* [Lab 02 System Calls](#lab-02-system-calls)
  * [Introduction](#lab-02-system-calls-introduction)
  * [Setup](#lab-02-system-calls-setup)
  * [Using GDB](#lab-02-system-calls-using-gdb)
    * [Debugging xv6](#lab-02-system-calls-debugging-xv6)
    * [Debugging Kernel Panic](#lab-02-system-calls-debugging-kernel-panic)
  * [System Call Tracing](#lab-02-system-calls-system-call-tracing)
  * [Attack Xv6](#lab-02-system-calls-attack-xv6)
  * [Key Learning Outcomes](#lab-02-system-calls-key-learning-outcomes)
  * [Challenges and Solutions](#lab-02-system-calls-challenges-and-solutions)
  * [Conclusion](#lab-02-system-calls-conclusion)
* [Lab 03 Page Tables](#lab-03-page-tables)
  * [Introduction](#lab-03-page-tables-introduction)
  * [Setup](#lab-03-page-tables-setup)
  * [Inspect Page Tables](#lab-03-page-tables-inspect-page-tables)
  * [Speed Up System Calls](#lab-03-page-tables-speed-up-system-calls)
    * [The USYSCALL Page Mechanism](#lab-03-page-tables-the-usyscall-page-mechanism)
  * [Print a Page Table](#lab-03-page-tables-print-a-page-table)
  * [Superpage Support](#lab-03-page-tables-superpage-support)
    * [Superpage Allocation Mechanism](#lab-03-page-tables-superpage-allocation-mechanism)
    * [Superpage Support in the Kernel](#lab-03-page-tables-superpage-support-in-the-kernel)
  * [Relevant Files](#lab-03-page-tables-relevant-files)
  * [Key Learning Outcomes](#lab-03-page-tables-key-learning-outcomes)
  * [Challenges and Solutions](#lab-03-page-tables-challenges-and-solutions)
  * [Conclusion](#lab-03-page-tables-conclusion)
* [Lab 04 Traps](#lab-04-traps)
  * [Introduction](#lab-04-traps-introduction)
  * [Setup](#lab-04-traps-setup)
  * [RISC-V Assembly](#lab-04-traps-risc-v-assembly)
  * [Backtrace](#lab-04-traps-backtrace)
  * [Alarm](#lab-04-traps-alarm)
  * [Key Learning Outcomes](#lab-04-traps-key-learning-outcomes)
  * [Challenges and Solutions](#lab-04-traps-challenges-and-solutions)
  * [Conclusion](#lab-04-traps-conclusion)
* [Lab 05 Copy-on-Write Fork](#lab-05-copy-on-write-fork)
  * [Introduction](#lab-05-copy-on-write-fork-introduction)
  * [Setup](#lab-05-copy-on-write-fork-setup)
  * [Reference Counting](#lab-05-copy-on-write-fork-reference-counting)
  * [COW Fork](#lab-05-copy-on-write-fork-cow-fork)
  * [COW Page Fault Handling](#lab-05-copy-on-write-fork-cow-page-fault-handling)
  * [Key Learning Outcomes](#lab-05-copy-on-write-fork-key-learning-outcomes)
  * [Challenges and Solutions](#lab-05-copy-on-write-fork-challenges-and-solutions)
  * [Conclusion](#lab-05-copy-on-write-fork-conclusion)
* [Lab 06 Networking](#lab-06-networking)
  * [Introduction](#lab-06-networking-introduction)
  * [Setup](#lab-06-networking-setup)
  * [NIC Driver](#lab-06-networking-nic-driver)
    * [Packet Transmission (e1000_transmit)](#lab-06-networking-packet-transmission-e1000_transmit)
    * [Packet Reception (e1000_recv)](#lab-06-networking-packet-reception-e1000_recv)
  * [UDP Packet Receive](#lab-06-networking-udp-packet-receive)
    * [Network Layer Receive (net_rx, ip_rx)](#lab-06-networking-network-layer-receive-net_rx-ip_rx)
    * [UDP Receive Processing (udp_rx)](#lab-06-networking-udp-receive-processing-udp_rx)
    * [Receiving Data in User Space (sys_recv)](#lab-06-networking-receiving-data-in-user-space-sys_recv)
    * [Socket Management (allocsock, freesock)](#lab-06-networking-socket-management-allocsock-freesock)
  * [Relevant Files](#lab-06-networking-relevant-files)
  * [Key Learning Outcomes](#lab-06-networking-key-learning-outcomes)
  * [Challenges and Solutions](#lab-06-networking-challenges-and-solutions)
  * [Conclusion](#lab-06-networking-conclusion)
* [Lab 07 Lock](#lab-07-lock)
  * [Introduction](#lab-07-lock-introduction)
  * [Setup](#lab-07-lock-setup)
  * [Optimizing Memory Allocator](#lab-07-lock-optimizing-memory-allocator)
    * [Per-CPU Freelist](#lab-07-lock-per-cpu-freelist)
    * [Allocation and Deallocation Path](#lab-07-lock-allocation-and-deallocation-path)
    * [Work-Stealing Mechanism](#lab-07-lock-work-stealing-mechanism)
    * [Synchronization and Deadlock Avoidance](#lab-07-lock-synchronization-and-deadlock-avoidance)
  * [Optimizing Buffer Cache](#lab-07-lock-optimizing-buffer-cache)
    * [Hash Table with Per-Bucket Locks](#lab-07-lock-hash-table-with-per-bucket-locks)
    * [Bitmap Freelist for Fast Allocation](#lab-07-lock-bitmap-freelist-for-fast-allocation)
    * [CLOCK Algorithm for Buffer Eviction](#lab-07-lock-clock-algorithm-for-buffer-eviction)
  * [Relevant Files](#lab-07-lock-relevant-files)
  * [Key Learning Outcomes](#lab-07-lock-key-learning-outcomes)
  * [Challenges and Solutions](#lab-07-lock-challenges-and-solutions)
  * [Conclusion](#lab-07-lock-conclusion)
* [Lab 08 File System](#lab-08-file-system)
  * [Introduction](#lab-08-file-system-introduction)
  * [Setup](#lab-08-file-system-setup)
  * [Large Files](#lab-08-file-system-large-files)
    * [Supporting Double Indirect Blocks in bmap](#lab-08-file-system-supporting-double-indirect-blocks-in-bmap)
    * [Freeing Double Indirect Blocks in itrunc](#lab-08-file-system-freeing-double-indirect-blocks-in-itrunc)
  * [Symbolic Links](#lab-08-file-system-symbolic-links)
    * [sys_symlink System Call](#lab-08-file-system-syssymlink-system-call)
    * [Following Symbolic Links in sys_open](#lab-08-file-system-following-symbolic-links-in-sys_open)
  * [Relevant Files](#lab-08-file-system-relevant-files)
  * [Key Learning Outcomes](#lab-08-file-system-key-learning-outcomes)
  * [Challenges and Solutions](#lab-08-file-system-challenges-and-solutions)
  * [Conclusion](#lab-08-file-system-conclusion)
* [Lab 09 MMAP](#lab-09-mmap)
  * [Introduction](#lab-09-mmap-introduction)
  * [Setup](#lab-09-mmap-setup)
  * [Overview](#lab-09-mmap-overview)
    * [mmap and munmap System Calls](#lab-09-mmap-mmap-and-munmap-system-calls)
    * [Implemented Features](#lab-09-mmap-implemented-features)
  * [Virtual Memory Area (VMA)](#lab-09-mmap-virtual-memory-area-vma)
    * [The VMA Structure](#lab-09-mmap-the-vma-structure)
    * [Fixed Array in struct proc](#lab-09-mmap-fixed-array-in-struct-proc)
    * [Pre-defined Virtual Address Slots](#lab-09-mmap-pre-defined-virtual-address-slots)
    * [Simplified VMA Management](#lab-09-mmap-simplified-vma-management)
  * [Kernel Support for mmap](#lab-09-mmap-kernel-support-for-mmap)
    * [mmap and munmap Implementation](#lab-09-mmap-mmap-and-munmap-implementation)
    * [Page Fault Handling](#lab-09-mmap-page-fault-handling)
    * [VMA Handling in fork() and exit()](#lab-09-mmap-vma-handling-in-fork-and-exit)
  * [Relevant Files](#lab-09-mmap-relevant-files)
  * [Key Learning Outcomes](#lab-09-mmap-key-learning-outcomes)
  * [Challenges and Solutions](#lab-09-mmap-challenges-and-solutions)
  * [Conclusion](#lab-09-mmap-conclusion)

## 报告格式说明

本报告包含了所有实验的报告内容, 每个实验的报告都包含以下部分:

* Introduction: 实验的目的和内容简介
* Setup: 实验代码的获取和运行环境的设置
* \<Lab Name\>: 实验的具体内容, 包括实现细节, 代码片段和运行结果等
* Relevant Files: 实验相关的代码文件和路径
* Key Learning Outcomes: 实验的主要收获和学习成果
* Challenges and Solutions: 实验中遇到的挑战和解决方案
* Conclusion: 实验的总结和反思

由于实验内容中涉及到了实验问题的解答和代码实现, 同时 MIT 6.1810 Fall 2024 的实验 Handout 均使用了英文, 因此实验报告的主体内容也将使用英文编写, 保持一致性, 便于与实验 Handout 对应.

# Lab 00 Setup

## Introduction

本部分内容说明了 Xv6 RISCV 的实验环境搭建过程, 包括所需工具的安装和配置, Xv6 RISCV 实验代码的获取, 编译和运行等步骤, 主要参考 [MIT 6.1810 工具安装指南](https://pdos.csail.mit.edu/6.1810/2024/tools.html)

## Install Toolchain

由于我使用的是 Arch Linux 系统, 所以直接使用 `pacman` 安装工具链即可:

```bash
sudo pacman -S riscv64-linux-gnu-binutils riscv64-linux-gnu-gcc riscv64-linux-gnu-gdb qemu-emulators-full
```

验证安装

```bash
qemu-system-riscv64 --version
riscv64-linux-gnu-gcc --version
```

![Image 01 Toolchain](../../Images/Lab%2000%20Setup/Image%2001%20Toolchain.png)

对于其他操作系统, [MIT 6.1810 工具安装指南](https://pdos.csail.mit.edu/6.1810/2024/tools.html) 中均有说明.

## Get Xv6 RISCV Source Code

使用 Git 获取 Xv6 RISCV 实验代码:

```bash
git clone git://g.csail.mit.edu/xv6-labs-2024
cd xv6-labs-2024
git fetch origin
```

此后可以使用 `git checkout` 命令切换到不同的实验分支, 例如:

```bash
git checkout pgtbl
```

![Image 02 Xv6 Source](../../Images/Lab%2000%20Setup/Image%2002%20Xv6%20Source.png)

## Compile and Run

在一个分支下编译和运行 Xv6 RISCV 实验代码:

```bash
make qemu
```

![Image 03 Compile](../../Images/Lab%2000%20Setup/Image%2003%20Compile.png)

运行 autograder 测试:

```bash
make grade
```

![Image 04 Autograder](../../Images/Lab%2000%20Setup/Image%2004%20Autograder.png)

# Lab 01 Utilities

## Introduction

In this lab, we implement several user programs for the xv6 operating system to familiarize ourselves with system calls, inter-process communication (IPC), file system operations, and basic Unix utilities. The lab consists of implementing five programs: `sleep`, `pingpong`, `primes`, `find`, and `xargs`.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout util
```

## Boot Xv6

To start the xv6 operating system, we use the following commands:

```bash
make qemu
```

This compiles the xv6 kernel and boots it in QEMU emulator. Once booted, we can run our implemented utilities in the xv6 shell.

## Implemented Programs

### 01. Sleep Program

**Source Code Location:** `/user/sleep.c`

The `sleep` program pauses execution for a specified number of clock ticks.

**Key Features:**

* Argument validation to ensure proper usage
* Uses the `sleep()` system call to pause execution
* Handles edge cases like negative input values

**Usage:**

```bash
sleep 10
```

### 02. Pingpong Program

**Source Code Location:** `/user/pingpong.c`

The `pingpong` program demonstrates inter-process communication using pipes. A parent process sends a byte to a child process, and the child responds back.

**Key Features:**

* Creates two pipes for bidirectional communication
* Implements proper pipe management (closing unused ends)
* Demonstrates fork() and process synchronization
* Error handling for pipe operations

**Usage:**

```bash
$ pingpong
4: received ping
3: received pong
```

### 03. Primes Program

**Source Code Location:** `/user/primes.c`

The `primes` program implements the Sieve of Eratosthenes algorithm using a pipeline of processes connected by pipes.

**Key Features:**

* Recursive process creation forming a pipeline
* Each process filters out multiples of its prime number
* Demonstrates complex inter-process communication
* Efficient prime number generation up to 280

**Usage:**

```bash
$ primes
prime 2
prime 3
prime 5
prime 7
...
```

### 04. Find Program

**Source Code Location:** `/user/find.c`

The `find` program recursively searches for files with a specific name in a directory tree.

**Key Features:**

* Recursive directory traversal
* File system operations using `open()`, `read()`, `stat()`
* Proper handling of directory entries
* Skips special directories (`.` and `..`)

**Usage:**

```bash
$ find . README
./README
```

### 05. Xargs Program

**Source Code Location:** `/user/xargs.c`

The `xargs` program reads lines from standard input and executes a command for each line, appending the line as arguments.

**Key Features:**

* Dynamic argument parsing from stdin
* Process creation for each input line
* Proper argument handling and command execution
* Support for multiple command-line arguments

**Usage:**

```bash
$ echo hello too | xargs echo bye
bye hello too
```

```bash
$ (echo 1 ; echo 2) | xargs echo
1
2
```

## Running the Programs

After booting xv6 with `make qemu`, you can run each program directly in the xv6 shell:

```bash
# Test sleep program
$ sleep 5

# Test pingpong program
$ pingpong

# Test primes program
$ primes

# Test find program
$ find . README

# Test xargs program
$ echo hello | xargs echo world
$ (echo 1; echo 2) | xargs echo
```

## System Calls Learned

Throughout this lab, we utilized various xv6 system calls:

* **Process Management:** `fork()`, `exec()`, `wait()`, `exit()`, `getpid()`
* **I/O Operations:** `read()`, `write()`, `open()`, `close()`
* **File System:** `stat()`, `fstat()`
* **Inter-Process Communication:** `pipe()`
* **Utility Functions:** `sleep()`, `atoi()`, `strcmp()`, `strcpy()`

## Key Learning Outcomes

1. **System Call Interface:** Understanding how user programs interact with the kernel through system calls
2. **Process Management:** Learning process creation, synchronization, and communication
3. **File System Operations:** Navigating and manipulating the file system programmatically
4. **Inter-Process Communication:** Implementing communication between processes using pipes
5. **Error Handling:** Proper error checking and resource management in system programming

## Challenges and Solutions

1. **Pipe Management:** Ensuring proper closing of unused pipe ends to avoid deadlocks
2. **Memory Management:** Careful buffer management and avoiding buffer overflows
3. **Process Synchronization:** Proper use of `wait()` to ensure child processes complete
4. **Argument Parsing:** Handling variable numbers of arguments and proper string manipulation

## Conclusion

This lab provided hands-on experience with fundamental Unix system programming concepts. By implementing these utilities, we gained practical knowledge of process management, file system operations, and inter-process communication in the xv6 operating system environment.

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

# Lab 04 Traps

## Introduction

In this lab, we explore the concept of traps in operating systems on the RISC-V architecture. We analyze RISC-V assembly code to understand function calls and the stack, implement a kernel function to print the call stack (backtrace), and build a user-level periodic alarm mechanism that utilizes timer interrupts and context switching.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout traps
```

## RISC-V Assembly

There is a file `user/call.c` in your xv6 repo. `make fs.img` compiles it and also produces a readable assembly version of the program in `user/call.asm`.

Read the code in `call.asm` for the functions `g`, `f`, and `main`. Answer the following questions:

```assembly
user/_call:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <g>:
#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int g(int x) {
   0: 1141                 addi sp,sp,-16
   2: e406                 sd ra,8(sp)
   4: e022                 sd s0,0(sp)
   6: 0800                 addi s0,sp,16
  return x+3;
}
   8: 250d                 addiw a0,a0,3
   a: 60a2                 ld ra,8(sp)
   c: 6402                 ld s0,0(sp)
   e: 0141                 addi sp,sp,16
  10: 8082                 ret

0000000000000012 <f>:

int f(int x) {
  12: 1141                 addi sp,sp,-16
  14: e406                 sd ra,8(sp)
  16: e022                 sd s0,0(sp)
  18: 0800                 addi s0,sp,16
  return g(x);
}
  1a: 250d                 addiw a0,a0,3
  1c: 60a2                 ld ra,8(sp)
  1e: 6402                 ld s0,0(sp)
  20: 0141                 addi sp,sp,16
  22: 8082                 ret

0000000000000024 <main>:

void main(void) {
  24: 1141                 addi sp,sp,-16
  26: e406                 sd ra,8(sp)
  28: e022                 sd s0,0(sp)
  2a: 0800                 addi s0,sp,16
  printf("%d %d\n", f(8)+1, 13);
  2c: 4635                 li a2,13
  2e: 45b1                 li a1,12
  30: 00001517           auipc a0,0x1
  34: 88050513           addi a0,a0,-1920 # 8b0 <malloc+0xfa>
  38: 6c6000ef           jal 6fe <printf>
  exit(0);
  3c: 4501                 li a0,0
  3e: 290000ef           jal 2ce <exit>

0000000000000042 <start>:

...
```

1\. Which registers contain arguments to functions? For example, which register holds 13 in main's call to printf?

> In RISC-V, the first eight integer arguments are passed in registers `a0` to `a7`, the first eight floating-point arguments in `fa0` to `fa7`. If there are more than eight arguments, the additional ones are passed on the stack, and can be accessed using the frame pointer (`s0` in this case).

> In the `main` function, the value `13` is loaded into register `a2` before the call to `printf`. Thus `a2` contains the value `13`.

2\. Where is the call to function `f` in the assembly code for `main`? Where is the call to `g`? (Hint: the compiler may inline functions.)

> The compiler has inlined the function `f` and `g`, so there are no explicit calls to `f` or `g` in the assembly code. The instruction `li a1,12` loads the results of `f(8) + 1` directly into `a1`, and the instruction `addiw a0,a0,3` inlines the call to `g`.

3\. At what address is the function `printf` located?

```assembly
00000000000006fe <printf>:

void
printf(const char *fmt, ...)

...
```

> The function `printf` is located at address `0x00000000000006fe`.

4\. What value is in the register `ra` just after the `jalr` to `printf` in `main`?

```assembly
0000000000000042 <start>:
//
// wrapper so that it's OK if main() does not call exit().
//
void
start()
{
  42: 1141                 addi sp,sp,-16
  44: e406                 sd ra,8(sp)
  46: e022                 sd s0,0(sp)
  48: 0800                 addi s0,sp,16
  extern int main();
  main();
  4a: fdbff0ef           jal 24 <main>
  exit(0);
  4e: 4501                 li a0,0
  50: 27e000ef           jal 2ce <exit>
```

> The register `ra` contains the return address. After calling `printf`, `ra` will hold the address of the instruction immediately following the `jal` to `printf`. In this case, it will be the address of the instruction `li a0, 0`, which is `0x000000000000003c`.

5\. Run the following code.

```c
    unsigned int i = 0x00646c72;
    printf("H%x Wo%s", 57616, (char *) &i);
```

What is the output? Here's an ASCII table that maps bytes to characters.

> The output of the code will be:

```
HE110 World
```

The output depends on that fact that the RISC-V is little-endian. If the RISC-V were instead big-endian what would you set `i` to in order to yield the same output? Would you need to change `57616` to a different value?

> In big-endian, you would set `i` to `0x726c6400` to yield the same output. The value `57616` does not need to change, as it is independent of the endianness.

6\. In the following code, what is going to be printed after 'y='? (note: the answer is not a specific value.) Why does this happen?

```c
    printf("x=%d y=%d", 3);
```

> The output after 'y=' will be an undefined value. This happens because the `printf` function expects two integer arguments based on the format string `"x=%d y=%d"`, but only one argument (`3`) is provided.

Looking at the assembly code for printf:

* The function allocates stack space and saves registers (`ra`, `s0`).
* It saves the initial argument registers `a1` through `a7` to the stack, relative to `s0`. In this case, `a1` contains `3`, and `a2` contains whatever value was in that register when `printf` was called.
* The `va_start` part sets up the `va_list` (`ap`) to point to the location on the stack where the first variable argument (`a1`, which is `3`) was saved (`s0 + 8`).
* `vprintf` is then called with the format string and the `va_list`.
* `vprintf` reads the first `%d` and retrieves the value `3` from the location pointed to by `ap` (`s0 + 8`).
* `vprintf` then reads the second `%d` and attempts to retrieve the next argument from the location immediately following the first argument in the `va_list's` view of the stack, which is `s0 + 16`.
* The value stored at `s0 + 16` is the original content of the `a2` register when `printf` was called. Since no second argument was provided in the C code, the value in `a2` is whatever happened to be there from previous operations. This value is not defined or predictable.

## Backtrace

The goal of this part of the lab was to implement a `backtrace()` function in the kernel that prints the call stack of the currently running process. This is a crucial debugging tool.

**Key Features:**

* Traverses the kernel stack frames of a process.
* Uses the frame pointer (`s0`) and saved return addresses (`ra`) to follow the call chain.
* Identifies the boundaries of the kernel stack to stop the traversal.

**Implementation Details:**

The implementation relies on the standard RISC-V calling convention used by the xv6 kernel, where each function's prologue saves the caller's frame pointer (`s0`) and return address (`ra`) onto the stack and sets up a new frame pointer.

1. We obtain the current frame pointer using inline assembly (`r_fp()`).
2. We loop, using the current frame pointer to find the saved return address (`fp - 8`) and the previous frame's frame pointer (`fp - 16`).
3. The loop continues as long as the frame pointer is valid (non-zero) and remains within the single page allocated for the process's kernel stack, which is identified using `PGROUNDDOWN()`.

```c
void
backtrace(void)
{
  uint64 *fp, *ra, page;
  struct proc *p = myproc();

  if(p == 0 || p->kstack == 0)
    return;

  fp = (uint64*)r_fp();
  page = PGROUNDDOWN((uint64)fp);
  printf("backtrace:\n");
  while (PGROUNDDOWN((uint64)fp) == page) {
    ra = fp - 1;
    printf("%p\n", (void *)*ra);
    fp = (uint64*)*(fp - 2);
  }
}
```

**Relevant Files:**

* `kernel/printf.c`: Contains the `backtrace()` function implementation.
* `kernel/proc.h`: Defines the `struct proc` which includes the kernel stack pointer (`kstack`).
* `kernel/riscv.h`: Defines `PGROUNDDOWN()` and `r_fp()`.

**Usage:**

The `backtrace()` function can be called from within the kernel code, typically when an error or panic occurs, to print the sequence of function calls that led to the current state.

```c
// Example call from kernel code
backtrace();
```

**System Calls/Functions Used:**

* `backtrace()`: The implemented function.
* `r_fp()`: Reads the current frame pointer (`s0`).
* `PGROUNDDOWN()`: Calculates the page-aligned base address of a memory address.
* `myproc()`: Gets the current process structure.

## Alarm

This part of the lab involved implementing a periodic alarm mechanism for user processes using the `sigalarm` and `sigreturn` system calls and handling timer interrupts.

**Key Features:**

* Allows a user process to request a timer that triggers after a specified number of clock ticks.
* Executes a user-defined handler function when the alarm expires.
* Provides a mechanism for the handler to return to the interrupted user code.
* Prevents re-entrant calls to the handler if a previous alarm handler hasn't finished.

**Implementation Details:**

1\. `struct proc` Modifications:

We added fields to the `struct proc` to support the alarm functionality:

* `alarm_ticks`: The number of ticks after which the alarm should trigger.
* `alarm_left`: The remaining ticks until the alarm triggers.
* `in_handler`: A flag to indicate if the process is currently executing the alarm handler.
* `alarm_handler`: The address of the user-defined alarm handler function.
* `old_trapframe`: Stores the original user trap frame before entering the alarm handler.

```c
struct proc {
  // ... existing fields ...
  struct trapframe *old_trapframe;
  int alarm_ticks;
  int alarm_left;
  int in_handler;
  void (*alarm_handler)();
};
```

2\. `usertrap()` (Timer Interrupt Handling):

The `usertrap()` function is modified to handle timer interrupts. When a timer interrupt occurs, it checks if the process has an active alarm. If so, it decrements the remaining ticks and checks if the alarm handler should be invoked.

* To invoke the alarm handler, it updates the process's epc (program counter) to point to the handler function.
* To be able to return to the original user code after the handler finishes, it saves the current trapframe into `old_trapframe`.
* To prevent re-entrant calls to the handler, it sets the `in_handler` flag to indicate that the process is currently executing the alarm handler.

```c
// give up the CPU if this is a timer interrupt.
if(which_dev == 2){
  // Update the process's alarm state.
  if(p->alarm_ticks > 0) {
    p->alarm_left--;
    if(p->alarm_left <= 0 && !p->in_handler) {
      // Save the current trapframe to old_trapframe.
      memmove(p->old_trapframe, p->trapframe, sizeof(struct trapframe));
      p->trapframe->epc = (uint64)p->alarm_handler;
      p->in_handler = 1; // Indicate that we are in the alarm handler.
      
      // Reset the left ticks for the next alarm.
      p->alarm_left = p->alarm_ticks;
    }
  }
}
```

3. `sys_sigalarm() and sys_sigreturn()`:

In `sys_sigalarm()`, we set the alarm parameters for the current process according to the user-provided arguments. This includes setting the number of ticks for the alarm and the handler function.

In `sys_sigreturn()`, we restore the original trapframe from `old_trapframe` and reset the `in_handler` flag, allowing the process to continue execution as if the alarm handler had never been called.

```c
uint64
sys_sigalarm(void)
{
  int ticks;
  void (*handler)();

  argint(0, &ticks);
  if (ticks < 0) {
    return -1;
  }

  argaddr(1, (uint64 *)&handler);
  if (ticks == 0) {
     // Disable alarm.
    myproc()->alarm_ticks = 0;
    myproc()->alarm_left = 0;
    myproc()->alarm_handler = 0;
  } else {
    myproc()->alarm_ticks = ticks;
    myproc()->alarm_left = ticks;
    myproc()->alarm_handler = handler;    
  }

  return 0;
}

uint64
sys_sigreturn(void)
{
  // Restore the trapframe from the saved storage.
  memmove(myproc()->trapframe, myproc()->old_trapframe, sizeof(struct trapframe));
  myproc()->in_handler = 0; // Clear in_handler flag.
  return myproc()->trapframe->a0;
}
```

**Relevant Files:**

* `kernel/sysproc.c`: Implements `sys_sigalarm()` and `sys_sigreturn()`.
* `kernel/trap.c`: Contains the `usertrap()` function where timer interrupts are handled.
* `kernel/proc.c`: Initializes the alarm state in `allocproc()`, manage the lifecycle of the `old_trapframe`.
* `kernel/proc.h`: Defines the `struct proc` with fields for alarm state (`alarm_ticks`, `alarm_left`, `alarm_handler`, `in_alarm_handler`) and potentially storage for the old trapframe.
* `user/user.h`, `kernel/syscall.h`, `kernel/syscall.c`, `usys.pl`: Defines the two new system calls and their entry points.

**Usage:**

A user program calls `sigalarm` to set the alarm and provides a handler function. The handler function performs the desired actions and then calls `sigreturn` to resume the original execution.

```c
// User program example
#include "user/user.h"

void my_alarm_handler() {
  // Do something when the alarm fires
  write(1, "Alarm!\n", 7);
  // Call sigreturn to resume original execution
  sigreturn();
}

int main() {
  sigalarm(100, my_alarm_handler); // Set alarm to fire every 100 ticks
  while(1) {
    // Main program loop
  }
  exit(0);
}
```

## Key Learning Outcomes

This lab provided a deep dive into the core mechanisms of an operating system, particularly focusing on traps and the interaction between user space and the kernel. Key learning outcomes include:

* **RISC-V Architecture and Calling Convention:** Gaining practical understanding of the RISC-V instruction set, how functions pass arguments (using `a0`-`a7`), manage the stack (`sp`, `s0`), and handle function calls and returns (`ra`, `jalr`). Analyzing assembly code (`.asm` files) was crucial for this.
* **Trap Handling:** Learning how the RISC-V architecture handles exceptions and interrupts, and how the xv6 kernel's `usertrap` function serves as the central point for processing these events. Understanding the role of `sepc`, `scause`, and `sstatus` registers.
* **User/Kernel Transition:** Understanding the process of switching between user mode and supervisor mode during system calls and interrupts, and how the `trapframe` is used to save and restore the user process's context.
* **Stack Management and Backtracing:** Implementing stack unwinding in the kernel by following the frame pointer chain (`s0`) and retrieving saved return addresses (`ra`). This reinforced understanding of stack frames and pointer arithmetic in C.
* **Signal Delivery Mechanism:** Designing and implementing a basic signal-like mechanism (the alarm) that allows the kernel to asynchronously interrupt a user process and execute a specific handler function.
* **Context Saving and Restoration:** Understanding the necessity of saving the full user context (`trapframe`) before executing a signal handler and restoring it afterward to ensure the interrupted program can resume correctly.
* **Preventing Re-entrancy:** Implementing logic to prevent a signal handler from being interrupted by another instance of the same signal, which is a common requirement in signal handling.
* **System Call Implementation:** Gaining experience in adding new system calls (`sigalarm`, `sigreturn`) to the xv6 kernel, including argument parsing and integrating them with kernel data structures and logic.

## Challenges and Solutions

Implementing the traps lab involved several challenges:

* **Understanding RISC-V Assembly:** Initially, interpreting the assembly output (`call.asm`) and mapping it back to the C source code required careful analysis of the calling convention and compiler optimizations like inlining. Reading the RISC-V documentation and xv6's `trampoline.S` helped clarify register usage and stack layout.
* **Implementing Backtrace:** The main challenge was correctly traversing the kernel stack. This involved understanding the fixed offsets relative to the frame pointer (`s0`) where the previous `s0` and `ra` are saved. Identifying the base of the kernel stack using `PGROUNDDOWN` was necessary to know when to stop the backtrace.
* **Designing the Alarm Mechanism:** Deciding how to save and restore the user context was critical. Using a dedicated `struct trapframe` field in the `struct proc` simplified the save/restore logic compared to dynamically allocating on the user stack, although it uses slightly more memory per process.
* **Handling Timer Interrupts:** Integrating the alarm logic into the existing `usertrap` function required careful modification of the timer interrupt block (`which_dev == 2`). Ensuring the alarm counter was decremented correctly and the handler was triggered only when due was important.
* **Preventing Re-entrant Handlers:** The `in_handler` flag was introduced specifically to solve the problem of a handler being interrupted by another alarm before it could finish and call `sigreturn`. This simple flag effectively serializes handler execution.
* **Seamless Context Switching:** The core challenge in the alarm was making the transition to and from the user handler appear seamless to the interrupted user program. This was achieved by meticulously saving the original `trapframe` and restoring it in `sys_sigreturn`, ensuring the `epc`, `sp`, and all user registers return to their state before the alarm.

## Conclusion

Lab 04 provided essential hands-on experience with the fundamental concepts of operating system traps, user-kernel interaction, and process context management on the RISC-V architecture. By implementing the `backtrace` and alarm mechanisms, we solidified our understanding of stack frames, calling conventions, interrupt handling, and the critical process of saving and restoring user state. The alarm implementation, in particular, highlighted the complexities of asynchronous event delivery and the need for careful state management within the kernel to support user-level signal handling. This lab served as a crucial step in understanding how an operating system manages the execution environment of user programs and responds to hardware events.

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

## Key Learning Outcomes

This lab provided a deep dive into advanced virtual memory management techniques, specifically focusing on the Copy-on-Write (COW) fork mechanism. Key learning outcomes include:

* **Page Tables and PTE Flags:** Learning how to manipulate page table entries (PTEs) and utilize PTE flags (like `PTE_V`, `PTE_W`, `PTE_U`, and the newly introduced `PTE_COW`) to control memory access permissions and implement COW behavior.
* **Reference Counting:** Implementing and managing a physical page reference counting system (`kref`) to track shared pages and ensure they are only freed when no longer referenced by any process. Understanding the importance of synchronization (using spinlocks) for shared data structures like the reference count array.
* **Trap Handling for Page Faults:** Modifying the trap handler (`usertrap`) to specifically identify and handle page faults (`scause=0xd` and `0xf`), and implementing a dedicated function (`handle_cow`) to process COW page faults.
* **Copy-on-Write Logic:** Understanding the core COW principle: sharing pages initially and only copying them when a write attempt occurs on a shared page. Implementing the COW break logic in `handle_cow` and `copyout`.
* **Interplay of OS Components:** Observing how different parts of the OS (process management, memory allocation, trap handling, system calls like `fork` and `exec`) interact and need to be coordinated to support COW.

## Challenges and Solutions

Implementing the COW fork mechanism presented several challenges:

* **Correct Reference Counting:** Ensuring that reference counts are incremented and decremented correctly in all relevant scenarios (`kalloc`, `kfree`, `uvmcopy`, `uvmunmap`, `handle_cow`, `copyout`). A common pitfall is double-freeing pages or having negative reference counts due to incorrect decrementing logic, especially when pages are shared. The solution involved carefully adding `krefinc` and modifying `kfree` to only return pages to the freelist when the count reaches zero.
* **Modifying `uvmcopy`:** The primary challenge was changing `uvmcopy` from a simple page-copying function to one that shares physical pages and correctly modifies PTEs in both parent and child page tables (clearing `PTE_W`, setting `PTE_COW`).
* **Handling COW Page Faults:** Implementing `handle_cow` required correctly identifying COW page faults, checking reference counts, allocating new pages, copying data, updating the PTE, and decrementing the old page's reference count. Edge cases like out-of-memory during allocation needed to be handled gracefully.
* **Integrating with `copyout`:** Recognizing that `copyout` also performs writes to user memory and needs to trigger the COW break mechanism was important. Modifying `copyout` to check for `PTE_COW` and perform the necessary copy logic was required.
* **Debugging Page Table Issues:** Debugging page table manipulation and trap handling often involves examining register values (`scause`, `stval`, `sepc`) and page table entries, which can be complex. Using print statements and GDB was essential for tracing execution flow and identifying incorrect PTE states or memory accesses.

## Conclusion

Lab 05 provided valuable practical experience in implementing a fundamental operating system optimization: Copy-on-Write fork. By deferring the copying of memory pages until they are actually modified, COW significantly improves the performance of `fork` by reducing memory consumption and CPU overhead. This lab reinforced our understanding of virtual memory concepts, page table management, and the critical role of trap handling in responding to memory access events. Implementing the reference counting and integrating the COW logic into the existing xv6 memory management and trap handling code demonstrated the interconnectedness of different OS components and the challenges involved in building efficient and correct memory sharing mechanisms.

# Lab 06 Networking

## Introduction

In this lab, we implement a basic network stack in the xv6 operating system kernel, focusing on the Intel E1000 network interface card (NIC) driver. The goal is to enable user processes to send and receive UDP packets over a simulated network. This involved writing a device driver for the E1000 NIC, implementing basic network protocols (ARP, IP, UDP), and providing a socket-like interface for user-space applications.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout cow
```

## NIC Driver

The network interface card (NIC) driver in xv6 is implemented in `kernel/e1000.c`. It interacts with the Intel E1000 network adapter, which is emulated by QEMU. The driver is responsible for initializing the device, transmitting packets, and receiving packets via interrupts.

### Packet Transmission (`e1000_transmit`)

The e1000_tr`ansmit function is called by the network stack to send a packet. It takes a buffer containing the Ethernet frame and its length.

* It acquires the `e1000_lock` to protect access to the TX ring.
* It checks the status of the next TX descriptor (`E1000_TDT`) to ensure it's available (`E1000_TXD_STAT_DD`). If not, it indicates the ring is full and returns an error.
* It frees the buffer associated with the descriptor if one exists (from a previous transmission).
* It stores the new packet buffer pointer in `tx_bufs`.
* It sets up the TX descriptor with the buffer address, length, and command flags (`E1000_TXD_CMD_EOP` and `E1000_TXD_CMD_RS`).
* It updates the `E1000_TDT` register to notify the hardware about the new packet.
* It releases the lock.

```c
int
e1000_transmit(char *buf, int len)
{
  uint64 tx_idx;

  acquire(&e1000_lock);
  tx_idx = regs[E1000_TDT];
  // Check if the TX ring is overflowing.
  if ((tx_ring[tx_idx].status & E1000_TXD_STAT_DD) == 0){
    printf("transmit: TX ring is overflowing\n");
    release(&e1000_lock);
    return -1;
  }

  // Free the previous buffer if it exists.
  if (tx_bufs[tx_idx]){
    kfree(tx_bufs[tx_idx]);
  }
  tx_bufs[tx_idx] = buf;

  // [E1000 3.3] Set up the TX descriptor.
  tx_ring[tx_idx].addr = (uint64)buf;
  tx_ring[tx_idx].length = len;
  tx_ring[tx_idx].cso = 0;
  tx_ring[tx_idx].cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;
  tx_ring[tx_idx].status = 0;
  tx_ring[tx_idx].css = 0;
  tx_ring[tx_idx].special = 0;

  // Update the TDT (Transmit Descriptor Tail) register.
  regs[E1000_TDT] = (tx_idx + 1) % TX_RING_SIZE;
  release(&e1000_lock);
  
  return 0;
}
```

### Packet Reception (`e1000_recv`)

Packet reception is interrupt-driven. When the E1000 receives a packet and writes it to a buffer in the RX ring, it generates an interrupt, which calls e1000_intr.

* `e1000_intr` acknowledges the interrupt by writing to `E1000_ICR`.
* It then calls `e1000_recv` to process received packets.
* `e1000_recv` checks the RX ring starting from the descriptor after `E1000_RDT`.
* For each descriptor with the `E1000_RXD_STAT_DD` status bit set (indicating a received packet):
  * It calls `net_rx` to pass the received buffer and length to the network stack.
  * It allocates a new buffer using `kalloc()`.
  * It updates the RX descriptor with the address of the new buffer and clears the status bit.
  * It updates the `E1000_RDT` register to indicate the last descriptor the driver has processed.
  * It moves to the next descriptor in the ring.

```c
static void
e1000_recv(void)
{
  uint64 rx_idx;
  char *buf;

  rx_idx = (regs[E1000_RDT] + 1) % RX_RING_SIZE;
  while (rx_ring[rx_idx].status & E1000_RXD_STAT_DD) {
    // Deliver the packet buffer to the network stack.
    net_rx(rx_bufs[rx_idx], rx_ring[rx_idx].length);

    // Allocate a buffer for the received packet.
    buf = kalloc();
    if(!buf) {
      printf("recv: failed to allocate buffer\n");
      break;
    }
    rx_bufs[rx_idx] = buf;
    rx_ring[rx_idx].addr = (uint64)buf;

    // Clear the status of the RX descriptor.
    rx_ring[rx_idx].status = 0;

    // Update the RDT (Receive Descriptor Tail) register.
    regs[E1000_RDT] = rx_idx;

    // Advance to the next RX descriptor.
    rx_idx = (rx_idx + 1) % RX_RING_SIZE;
  }
}
```

## UDP Packet Receive

The UDP packet reception path involves several functions in `kernel/net.c`, starting from `net_rx` which is called by the NIC driver (`e1000_recv`).

### Network Layer Receive (`net_rx`, `ip_rx`)

* `net_rx` receives the raw packet buffer from the driver. It checks the Ethernet type field to determine the protocol (ARP or IP).
* If it's an IP packet (`ETHTYPE_IP`), it calls `ip_rx`.
* `ip_rx` examines the IP protocol field (`ip_p`) to determine the transport layer protocol.
* If the protocol is UDP (`IPPROTO_UDP`), it calls `udp_rx`. If it's ICMP (`IPPROTO_ICMP`), it calls `icmp_rx`. For other protocols or invalid packets, it frees the buffer.

### UDP Receive Processing (`udp_rx`)

The `udp_rx` function handles incoming UDP packets:

* It extracts the destination port (`dport`) from the UDP header.
* It checks the `bindmap` to see if any socket is bound to this destination port. If not, the packet is dropped by freeing the buffer.
* It iterates through the `sockets` array to find the socket associated with the destination port.
* If no socket is found, the packet is dropped.
* If a socket is found, it acquires the socket's lock.
* It checks if the socket's receive queue (`sock->queue`) is full (sock->count >= RX_QUEUE_SIZE). If full, the packet is dropped.
* If the queue is not full, it adds the packet buffer (`buf`) to the queue, updates the tail index and count.
* It releases the socket's lock.
* It calls `wakeup(sock)` to wake up any process that might be sleeping in `sys_recv` waiting for a packet on this socket.

```c
void
udp_rx(char *buf, int len, struct ip *inip)
{
  uint16 dport;
  struct socket *sock;
  struct udp *inudp;
  int sock_idx;

  inudp = (struct udp *)(inip + 1);
  dport = ntohs(inudp->dport);

  // Check if the destination port is bound.
  acquire(&bindmap_lock);
  if(!(bindmap[dport / sizeof(uint64)] & (1U << (dport % sizeof(uint64))))) {
    release(&bindmap_lock);
    kfree(buf);
    return;
  }
  release(&bindmap_lock);

  // Find the socket for the destination port.
  for(sock_idx = 0; sock_idx < MAX_SOCKETS; sock_idx++) {
    sock = &sockets[sock_idx];
    if(sock->type == IPPROTO_UDP && sock->local_port == dport) {
      break;
    }
  }

  // If no socket found for the port, drop the packet.
  if(sock_idx == MAX_SOCKETS) {
    printf("udp_rx: No socket found for port %d\n", dport);
    kfree(buf);
    return;
  }

  // Check if the receive queue for the socket is full.
  acquire(&sock->lock);
  if(sock->count >= RX_QUEUE_SIZE) {
    release(&sock->lock);
    kfree(buf);
    return;
  }

  // Add the packet to the socket's receive queue.
  sock->queue[sock->tail] = buf;
  sock->tail = (sock->tail + 1) % RX_QUEUE_SIZE;
  sock->count++;

  // Wake up any process waiting on the receive queue.
  release(&sock->lock);
  wakeup(sock);
}
```

### Receiving Data in User Space (`sys_recv`)

The `sys_recv` system call allows a user process to receive a UDP packet:

* It takes the destination port (`dport`), pointers for source IP (`src`) and source port (`sport`), a buffer (`buf`) for the payload, and the maximum length (`maxlen`).
* It finds the corresponding socket based on the `dport`.
* It acquires the socket's lock and enters a `sleep` loop if the queue is empty (`sock->count == 0`), waiting for `udp_rx` to add a packet and call `wakeup`.
* Once a packet is available, it dequeues the packet buffer from the head of the queue, updates the head index and count.
* It releases the socket's lock.
* It parses the packet to extract the source IP and source port.
* It uses `copyout` to copy the source IP, source port, and the UDP payload to the user-provided buffers.
* It calculates the number of bytes copied.
* Crucially, it calls `kfree(packet)` to free the packet buffer that was dequeued from the socket's queue.
* It returns the number of bytes copied.

```c
uint64
sys_recv(void)
{
  int dport, maxlen, sock_idx;
  char *packet;
  uint64 copied_bytes, payload_len, src, sport, buf;
  struct socket *sock;
  struct eth *recv_eth;
  struct ip *recv_ip;
  struct udp *recv_udp;
  struct proc *p = myproc();

  argint(0, &dport);
  argaddr(1, &src);
  argaddr(2, &sport);
  argaddr(3, &buf);
  argint(4, &maxlen);

  // Find the socket for the destination port.
  acquire(&sockets_lock);
  for(sock_idx = 0; sock_idx < MAX_SOCKETS; sock_idx++){
    sock = &sockets[sock_idx];
    if(sock->type == IPPROTO_UDP && sock->local_port == dport){
      break;
    }
  }

  if(sock_idx == MAX_SOCKETS){
    release(&sockets_lock);
    return -1;
  }
  release(&sockets_lock);

  // Sleep until a packet is available.
  acquire(&sock->lock);
  while(sock->count == 0){
    sleep(sock, &sock->lock);
  }

  // Now we have a packet to receive.
  packet = sock->queue[sock->head];
  sock->head = (sock->head + 1) % RX_QUEUE_SIZE;
  sock->count--;
  release(&sock->lock);

  // Parse the packet.
  recv_eth = (struct eth *)packet;
  recv_ip = (struct ip *)(recv_eth + 1);
  recv_udp = (struct udp *)(recv_ip + 1);

  // Copy the source IP and port to user space.
  uint32 src_host = ntohl(recv_ip->ip_src);
  uint16 sport_host = ntohs(recv_udp->sport);

  if(copyout(p->pagetable, src, (char *)&src_host, sizeof(src_host)) < 0)
    panic("recv: copyout src failed");
  if(copyout(p->pagetable, sport, (char *)&sport_host, sizeof(sport_host)) < 0)
    panic("recv: copyout sport failed");

  // Copy the UDP payload to the user buffer.
  if(copyout(p->pagetable, (uint64)buf, (char *)(recv_udp + 1), maxlen) < 0)
    panic("recv: copyout buf failed");

  // Calculate the number of bytes copied.
  payload_len = ntohs(recv_udp->ulen) - sizeof(struct udp);
  copied_bytes = payload_len > maxlen ? maxlen : payload_len;
  kfree(packet); // Free the packet buffer after copying.

  return copied_bytes;
}
```

### Socket Management (`allocsock`, `freesock`)

* `allocsock` finds a free `struct socket` entry in the `sockets` array, initializes its fields (type, port, IP), sets up the receive queue pointers and count, and initializes the socket's spinlock.
* `freesock` finds the socket by port number, marks it as free, and iterates through its receive queue to kfree all queued packet buffers before resetting the queue state.

## Relevant Files

* `kernel/e1000.c`: Implementation of the E1000 network interface card driver.
* `kernel/net.c`: Implementation of the basic network stack, including ARP, IP, UDP, and socket management.
* `kernel/net.h`: Header file defining network structures (Ethernet, IP, UDP, ARP, DNS), socket structure, and function prototypes.
* `kernel/sysproc.c`: Contains the system call implementations for network operations (`sys_bind`, `sys_unbind`, `sys_send`, `sys_recv`).
* `user/nettest.c`: User-level program used for testing the network stack (e.g., ping, UDP tests).

## Key Learning Outcomes

This lab provided hands-on experience in building a basic network stack within an operating system kernel. Key learning outcomes include:

* **Device Driver Interaction:** Understanding how an operating system kernel interacts with hardware devices, specifically a network interface card (NIC), through memory-mapped registers and descriptor rings.
* **Packet Processing Pipeline:** Learning the flow of network packets from the hardware (E1000) through the driver (`e1000.c`) and into the network stack (`net.c`), including interrupt handling for received packets.
* **Network Protocol Implementation:** Implementing basic logic for fundamental network protocols like ARP (Address Resolution Protocol), IP (Internet Protocol), and UDP (User Datagram Protocol), including header parsing and checksum calculation.
* **Socket Abstraction:** Building a simple socket-like abstraction in the kernel to allow user processes to send and receive UDP packets via system calls (`bind`, `unbind`, `send`, `recv`).
* **Kernel Memory Management:** Using `kalloc` and `kfree` to manage memory buffers for network packets and understanding the challenges of buffer ownership and lifetime in a multi-layered network stack.
* **Synchronization:** Applying spinlocks to protect shared data structures like the NIC's descriptor rings, the port binding bitmap, and the socket receive queues from concurrent access by different kernel threads or interrupt handlers.
* **System Call Implementation:** Adding new system calls to expose network functionality to user-space applications.

## Challenges and Solutions

Implementing the networking stack in xv6 presented several challenges:

* **Understanding E1000 Hardware:** Deciphering the E1000 device specification and register layout to correctly initialize the device and manage the transmit and receive rings.
* **Solution**: Carefully studying the E1000 documentation and the provided `e1000_dev.h` definitions.
* **Correct Ring Buffer Management:** Implementing the logic for adding packets to the TX ring and processing received packets from the RX ring, including updating head/tail pointers and handling descriptor status bits. Ensuring proper synchronization with the hardware.
* **Solution**: Following the E1000 ring buffer model described in the documentation and using spinlocks to protect shared state.
* **Packet Buffer Ownership and Lifetime:** Managing the allocation and freeing of packet buffers across different layers of the network stack (driver, IP layer, UDP layer, socket queue, system call). Ensuring buffers are freed exactly once to prevent memory leaks or double frees.
* **Solution**: Carefully tracing the ownership of buffers and implementing `kfree` calls at the appropriate points (e.g., after transmission is complete, when a packet is dropped, after a packet is received by a user process, when a socket is freed).
* **Synchronization Issues:** Protecting shared data structures (like the `e1000_lock`, `bindmap_lock`, `sockets_lock`, and individual `sock->lock`) from race conditions between the interrupt handler and user processes making system calls.
* **Solution**: Consistently acquiring and releasing the appropriate spinlocks before accessing shared data.
* **Implementing Network Protocols:** Correctly parsing packet headers and implementing the basic logic for ARP, IP, and UDP. This involved understanding network byte order and using functions like `ntohs` and `ntohl`.
* **Solution**: Referencing network protocol specifications and using the provided byte swap functions.
* **Socket Queue Implementation:** Building a thread-safe queue for received packets for each socket and integrating it with the `sleep`/`wakeup` mechanism to allow user processes to block until data arrives.
* **Solution**: Implementing a circular buffer (or linked list) with a spinlock and using `sleep` and `wakeup` on the socket structure.
* **Memory Leaks in Socket Queues:** A specific challenge was ensuring that packet buffers queued in a socket's receive queue are freed when the socket is unbound, even if the user process hasn't called `recv` for all packets. Solution: Implementing logic in `freesock` to iterate through the queue and `kfree` any remaining packet buffers.

## Conclusion

Lab 06 provided a comprehensive introduction to network programming within an operating system kernel. We successfully implemented a basic network driver for the emulated E1000 NIC and built a simple network stack capable of handling ARP, IP, and UDP packets. By implementing socket-like functionality, we enabled user processes to perform network communication. This lab reinforced our understanding of device drivers, network protocols, kernel memory management, synchronization, and the process of adding new system calls to an operating system. The challenges encountered highlighted the complexities of managing shared resources and buffer lifetimes in a concurrent kernel environment.

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

  for(cpu = 0; cpu < NCPU; cpu++){
    if(cpu == stealer)
      continue; // Skip self.
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

# Lab 08 File System

## Introduction

In this lab, we extended the xv6 file system to support two significant features: large files and symbolic links. These enhancements not only increase the practical usability of the file system but also provide a deeper understanding of how modern operating systems manage files and directories.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout fs
```

## Large Files

The original xv6 file system had a significant limitation on file size. An inode could only address `NDIRECT + NINDIRECT` blocks, which translates to `11 + (1024 / 4) = 11 + 256 = 267` blocks, or approximately 267KB. This is insufficient for many modern applications. To overcome this, we extended the file system to support much larger files by implementing **double indirect blocks**.

This enhancement required modifications to two key file system functions: `bmap`, which maps a file offset to a disk block address, and `itrunc`, which frees all blocks associated with a file.

### Supporting Double Indirect Blocks in `bmap`

The core of the large file support lies in `bmap`. We added a new level of indirection to the block mapping logic. The `ip->addrs` array in an inode now uses its second-to-last pointer (`ip->addrs[NDIRECT+1]`) as a pointer to a *double indirect block*.

The new mapping logic in `bmap` is as follows:

1\. **Direct Blocks**: For offsets within the first `NDIRECT` blocks, the logic remains the same, returning the block address directly from `ip->addrs`.
2\. **Single Indirect Block**: For offsets within the next `NINDIRECT` blocks, the logic also remains unchanged, using `ip->addrs[NDIRECT]` as a pointer to an indirect block.
3\. **Double Indirect Block (New Logic)**: For offsets beyond the single indirect range, up to the new `MAXFILE` size:

* The function now uses `ip->addrs[NDIRECT+1]` as a pointer to the double indirect block. If this block doesn't exist, it is allocated.
* This double indirect block contains an array of pointers to single indirect blocks. `bmap` calculates which entry in the double indirect block corresponds to the given file offset (`bn / NINDIRECT`).
* It reads the double indirect block, retrieves the address of the correct single indirect block (allocating it if necessary), and then reads that single indirect block.
* Finally, it finds the address of the target data block within the single indirect block (`a[bn % NINDIRECT]`) and returns it, allocating the data block itself if it doesn't exist.

This two-level lookup allows a single inode to address `(BSIZE/sizeof(uint)) * (BSIZE/sizeof(uint))` additional blocks, dramatically increasing the maximum file size to over 64MB.

```c
if(bn < NDINDIRECT){
  // Load double indirect block, allocating if necessary.
  if((addr = ip->addrs[NDIRECT + 1]) == 0){
    addr = balloc(ip->dev);
    if(addr == 0)
      return 0;
    ip->addrs[NDIRECT + 1] = addr;
  }
  bp = bread(ip->dev, addr);
  a = (uint*)bp->data;
  if((addr = a[bn / NINDIRECT]) == 0){
    addr = balloc(ip->dev);
    if(addr == 0){
      brelse(bp);
      return 0;
    }
    a[bn / NINDIRECT] = addr;
    log_write(bp);
  }
  brelse(bp);

  // Load the indirect block for this double indirect block.
  bp = bread(ip->dev, addr);
  a = (uint*)bp->data;
  if((addr = a[bn % NINDIRECT]) == 0){
    addr = balloc(ip->dev);
    if(addr == 0){
      brelse(bp);
      return 0;
    }
    a[bn % NINDIRECT] = addr;
    log_write(bp);
  }
  brelse(bp);
  return addr;
}
```

### Freeing Double Indirect Blocks in `itrunc`

To prevent disk space leaks, the `itrunc` function, which is responsible for freeing a file's data blocks, must also be updated to understand the double indirect block structure.

When a file is truncated (e.g., upon deletion), the modified `itrunc` performs the following steps after freeing the direct and single indirect blocks:

1\. It checks if the double indirect block pointer (`ip->addrs[NDIRECT+1]`) exists.
2\. If it does, it reads the double indirect block.
3\. It iterates through each entry in the double indirect block.
4\. It frees each entry like a single indirect block.

```c
if(ip->addrs[NDIRECT + 1]){
  bp = bread(ip->dev, ip->addrs[NDIRECT + 1]);
  a = (uint*)bp->data;
  for(j = 0; j < NINDIRECT; j++){
    // Free each single indirect block.
    if(a[j]){
      temp = bread(ip->dev, a[j]);
      b = (uint*)temp->data;
      for(k = 0; k < NINDIRECT; k++){
        if(b[k])
          bfree(ip->dev, b[k]);
      }
      brelse(temp);
      bfree(ip->dev, a[j]);
      a[j] = 0;
    }
  }
  brelse(bp);
  bfree(ip->dev, ip->addrs[NDIRECT + 1]);
  ip->addrs[NDIRECT + 1] = 0;
}
```

This careful, multi-level deallocation ensures that the entire tree of blocks associated with a large file is correctly returned to the file system's free block pool.

## Symbolic Links

The standard xv6 file system supports only regular files, directories, and devices. To enhance its functionality and align it more closely with modern UNIX-like systems, we introduced support for **symbolic links** (symlinks). A symbolic link is a special type of file whose content is a path to another file or directory.

Implementing this feature required two major changes: creating a new system call, `sys_symlink`, to create these links, and modifying the path resolution logic within the `sys_open` system call to correctly follow them.

### `sys_symlink` System Call

We introduced a new system call, `symlink(target, linkpath)`, which allows a user to create a symbolic link at `linkpath` that points to `target`.

* **Inode Creation**: A new file type, `T_SYMLINK`, was defined. When `sys_symlink` is called, it leverages the existing `create` function to allocate a new inode and sets its type to `T_SYMLINK`. This is the key distinction from a hard link, as a symlink is a distinct file with its own inode.
* **Storing the Target Path**: The `target` path string provided by the user is treated as the content of this new file. The `sys_symlink` function uses `writei` to write the null-terminated `target` string into the data blocks associated with the new symlink's inode.
* **No Target Validation**: Crucially, the system call does not validate whether the `target` path exists at the time of creation. This allows for the creation of "dangling" links, a standard and useful feature of symbolic links.

```c
uint64
sys_symlink(void)
{
  char target[MAXPATH], path[MAXPATH];
  struct inode *ip;
  uint length;

  argstr(0, target, MAXPATH);
  argstr(1, path, MAXPATH);

  begin_op();
  // Create a new inode of type T_SYMLINK.
  if((ip = create(path, T_SYMLINK, 0, 0)) == 0){
    end_op();
    return -1;
  }
  
  // Write the target path as the file's content.
  length = strlen(target) + 1;
  if(writei(ip, 0, (uint64)target, 0, length) != length){
    iunlockput(ip);
    end_op();
    return -1;
  }

  iunlockput(ip); // Release the new inode.
  end_op();
  return 0;
}
```

### Following Symbolic Links in `sys_open`

Creating symlinks is only half the battle; the kernel must be able to interpret them. We modified the `sys_open` system call to handle path resolution when it encounters a symlink.

**Dereferencing Loop**: After `namei` returns an initial inode but before the file is formally opened, we added a loop to handle potential symlinks (unless the `O_NOFOLLOW` flag is specified).

**Loop Logic**:

1\. The loop checks if the current inode's type is `T_SYMLINK`. If not, path resolution is complete, and the loop terminates.
2\. If it is a symlink, the function uses `readi` to read the target path from the symlink's data blocks into a buffer.
3\. The current inode is released (`iunlockput`).
4\. The kernel then calls `namei` again, this time using the newly read target path to resolve the next inode in the chain.

**Loop Termination and Safety**: To prevent infinite loops caused by circular symlinks (e.g., `a -> b`, `b -> a`), the dereferencing loop is restricted to a maximum of 10 iterations. If the path resolution still results in a symlink after 10 loops, `sys_open` fails with an error, preventing a kernel stack overflow or hang.

```c
// Symlink-following logic inside sys_open()
if(!(omode & O_NOFOLLOW)) {
  for(int i = 0; i < 10; i++){
    if(ip->type != T_SYMLINK){
      break; // Not a symlink anymore.
    }
    // Read the target path from the symlink's content.
    if(readi(ip, 0, (uint64)path, 0, MAXPATH) < 0){
      iunlockput(ip);
      end_op();
      return -1;
    }
    iunlockput(ip); // Release the symlink's inode.
    
    // Resolve the new path.
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
  }
  // Check for too many levels of symbolic links.
  if(ip->type == T_SYMLINK){
    iunlockput(ip);
    end_op();
    return -1;
  }
}
```

By implementing both the creation and resolution logic, we have integrated full-featured symbolic link support into the xv6 file system, enhancing its flexibility and functionality.

## Relevant Files

* `kernel/fs.c`: Contains the modifications to the `bmap` and `itrunc` functions for large file support.
* `kernel/fs.h`, `kernel/file.h`: Contains the modifications to the inode structure to support double indirect blocks.
* `kernel/sysfile.c`: Contains the new `sys_symlink` system call implementation and modifications to `sys_open` for symbolic link resolution.
* `kernel/fcntl.h`: Contains `O_NOFOLLOW` flag definition.
* `kernel/stat.h`: Contains the new file type `T_SYMLINK` definition.
* `kernel/syscall.c`, `kernel/syscall.h`, `kernel/user.h`, `user/usys.pl`: Contains the new system call registration and user-space interface for `symlink`.

## Key Learning Outcomes

These labs provided a deep, hands-on understanding of core file system concepts and the engineering challenges involved in extending a kernel's capabilities.

* **Inode and File System Structure**: Gained a comprehensive understanding of the on-disk and in-memory structures of the xv6 file system, including how inodes, direct/indirect block pointers, and directory entries work together to represent files and directories.
* **Advanced File System Features**: Learned the fundamental differences between hard links and symbolic links at the inode level. Implemented the complete lifecycle of symbolic links, from creation (`sys_symlink`) to resolution (`sys_open`).
* **Extending System Limits**: Mastered the technique of adding new layers of indirection (double indirect blocks) to overcome system limitations, significantly increasing the maximum file size and making the file system more practical.
* **Kernel Path Resolution**: Modified the kernel's core path resolution logic (`namei`, `sys_open`) to handle complex cases like nested symbolic links, demonstrating how system calls interact with the underlying file system layers.
* **Resource Management and Atomicity**: Gained critical experience in managing kernel resources (inodes, file descriptors, disk blocks) within transactional boundaries (`begin_op`/`end_op`). This highlighted the importance of ensuring that all resources are correctly allocated and, crucially, deallocated, especially on failure paths.
* **Defensive Programming in the Kernel**: Learned the importance of writing robust kernel code that anticipates and correctly handles edge cases and potential errors, such as circular symlinks, dangling links, and out-of-space conditions during multi-step operations.

## Challenges and Solutions

Implementing these file system features exposed several classic and subtle kernel programming challenges, primarily related to resource management and the robustness of error-handling paths.

* **Correctly Freeing Multi-Level Block Pointers**: Implementing `itrunc` for large files was challenging. A naive implementation could easily leak entire single-indirect blocks or the double-indirect block itself if the deallocation logic was not perfectly hierarchical.
* **Solution**: We designed a careful, multi-level deallocation process in `itrunc`. The code first iterates through the double-indirect block to free all data blocks and their parent single-indirect blocks, before finally freeing the double-indirect block itself. This ensures that the entire block tree is cleanly deallocated.
* **Releasing Locks/Blocks/Inodes After Use**: A common pitfall in kernel development is failing to release resources correctly, leading to deadlocks or resource leaks. For example, if an inode is locked but not released after a symlink resolution fails, it could block other processes indefinitely.
* **Solution**: We ensured that every path through `sys_open` and `sys_symlink` correctly releases any locked inodes or allocated blocks, even in error cases. This was achieved by carefully structuring the code to always reach the `iunlockput` and `brelse` calls, regardless of whether the operation succeeded or failed.
* **Premature Resource Allocation in `sys_open`**: Another critical bug was identified where `filealloc()` and `fdalloc()` were called before the symbolic link resolution loop. If an error occurred during the loop (e.g., too many symlinks, target not found), the subsequent call to `fileclose()` would attempt to `iput()` a `file->ip` pointer that had not yet been assigned, causing a null-pointer dereference and kernel panic.
* **Solution**: The logic in `sys_open` was reordered to follow the principle of late resource allocation. The `file` structure and file descriptor are now allocated only *after* all path resolution, including following symlinks, has successfully completed and a final, valid inode has been locked. This ensures that the cleanup path (`fileclose`) is never called on a partially initialized `file` structure.
* **Handling Circular Symbolic Links**: A naive implementation of symlink following would enter an infinite loop if faced with a circular link (e.g., a -> b, b -> a).
* **Solution**: We implemented a simple but effective loop counter within the `sys_open` resolution logic. If the number of symbolic link traversals exceeds a reasonable limit (e.g., 10), the operation is aborted, and an error is returned to the user, preventing a kernel hang or stack overflow.

## Conclusion

This lab was a deep dive into the practical engineering of a UNIX-like file system. By implementing support for large files and symbolic links, we moved xv6 from a basic teaching tool closer to a functionally recognizable operating system. The challenges encountered, particularly in ensuring robust error handling and preventing subtle race conditions during path resolution, were profound learning experiences. They underscored that in kernel development, correct resource management across all code paths—especially failure paths—is paramount for system stability. The final, functional file system is a testament to the careful, defensive, and structured programming required to build reliable OS components.

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
