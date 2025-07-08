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

In this lab, we will

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

Looking at the backtrace output, which function called `syscall` ?

> `usertrap`

What is the value of `p->trapframe->a7` and what does that value represent? (Hint: look `user/initcode.S`, the first user program xv6 starts.)

> 7, `SYS_exit`, which is the system call number for `exit`.

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

Corresponding assembly codes in `kernel/kernel.asm`:

```assembly
  struct proc *p = myproc();
    80002a0c:	a9dfe0ef          	jal	800014a8 <myproc>
    80002a10:	84aa                	mv	s1,a0

  num = * (int *) 0;
    80002a12:	00002683          	lw	a3,0(zero) # 0 <_entry-0x80000000>
```

Write down the assembly instruction the kernel is panicing at. Which register corresponds to the variable `num`?

> `lw a3,0(zero)`, where `a3` corresponds to the variable `num`.

Why does the kernel crash? Hint: look at figure 3-3 in the text; is address 0 mapped in the kernel address space? Is that confirmed by the value in `scause` above? (See description of `scause` in RISC-V privileged instructions)

The `scause` value is `0xd` (decimal 13), which indicates a "Load Page Fault" exception according to the RISC-V privileged specification.

![Memory Map](../../Images/Lab%2002%20System%20Calls/image.png)

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

## System Call Tracing
