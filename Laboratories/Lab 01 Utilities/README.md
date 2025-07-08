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

# Lab 01 Utilities

## Introduction

In this lab, we implemented several user programs for the xv6 operating system to familiarize ourselves with system calls, inter-process communication (IPC), file system operations, and basic Unix utilities. The lab consists of implementing five programs: `sleep`, `pingpong`, `primes`, `find`, and `xargs`.

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
