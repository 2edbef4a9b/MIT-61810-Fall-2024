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
