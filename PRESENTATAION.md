---
description: MIT 6.1810 Fall 2024 Xv6 RISC-V 实验项目
---

# Xv6 RISC-V

[https://pdos.csail.mit.edu/6.1810/2024/](https://pdos.csail.mit.edu/6.1810/2024/)

## 项目简介

xv6 是 MIT 开发的教学操作系统, 基于 RISC-V 架构, 一共有 9 个实验项目:

1. Lab 01: Utilities
2. Lab 02: System Calls
3. Lab 03: Page Tables
4. Lab 04: Traps
5. Lab 05: Copy-on-Write Fork
6. Lab 06: Networking
7. Lab 07: Locks
8. Lab 08: File System
9. Lab 09: MMAP

实验项目从基本的 Unix 工具实现到复杂的网络栈, 内存管理和文件系统, 逐步深入操作系统的核心组件.

## 项目基本信息

### 实验环境与工具链

* 使用 RISC-V 工具链: `riscv64-linux-gnu-gcc`, `qemu-system-riscv64`
* 开发环境: Arch Linux 系统
* 代码管理: Git 分支管理, 每个 Lab 对应独立分支

### 项目地址

仓库地址: [https://github.com/2edbef4a9b/MIT-61810-Fall-2024](https://github.com/2edbef4a9b/MIT-61810-Fall-2024)

通过 Git 获取代码:

```bash
git clone --recursive https://github.com/2edbef4a9b/MIT-61810-Fall-2024.git
```

### 项目结构

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

其中 `Xv6-RISCV` 是一个 submodule, 地址为 [https://github.com/2edbef4a9b/Xv6-RISCV-Assignment](https://github.com/2edbef4a9b/Xv6-RISCV-Assignment), 包含了 Xv6 RISCV 实验的源代码和相关文件, 会在运行 `git clone --recursive` 时自动获取, submodule 中包含每个 Lab 对应的分支, 要获取特定实验的代码, 可以使用 `git checkout` 命令切换到对应的分支, 例如:

```bash
cd Xv6-RISCV
git checkout pgtbl
```

## 实验内容

### Lab 00: Setup (实验环境搭建)

在 Arch Linux 下搭建实验环境, 安装工具链, 获取 Xv6 源代码, 编译和运行实验代码

```bash
sudo pacman -S riscv64-linux-gnu-binutils riscv64-linux-gnu-gcc \
    riscv64-linux-gnu-gdb qemu-emulators-full

git clone git://g.csail.mit.edu/xv6-labs-2024
cd xv6-labs-2024
git fetch origin
git checkout util
make qemu
```

### Lab 01: Utilities (Unix 工具实现)

**目标**: 熟悉 xv6 系统调用和 Unix 编程

* `sleep`: 基本系统调用接口
* `pingpong`: 进程创建和管道通信
* `primes`: 通过 IPC 实现并发素数筛算法
* `find`: 文件系统遍历和递归搜索
* `xargs`: 参数处理和程序执行

**核心收获**: 理解进程管理, IPC 机制, 管道资源管理, 文件系统操作.

### Lab 02: System Calls (系统调用实现)

**目标**: 深入理解系统调用机制

* 使用 GDB 调试内核和用户程序
* 实现 `trace` 系统调用追踪功能
* 探索内存安全漏洞并编写 exploit

**核心收获**:

* 掌握系统调用的完整实现流程
* 学会内核级调试技术
* 理解安全漏洞的原理和利用

### Lab 03: Page Tables (页表管理)

**目标**: 深入虚拟内存管理

* 理解页表项的格式和权限位
* `USYSCALL` 页: 优化系统调用数据传输
* `vmprint`: 实现页表结构可视化
* 超级页面支持: 实现 2MB 页面映射

**核心挑战**:

* 理解 RISC-V 页表的三级结构
* 实现支持超级页面的伙伴系统

### Lab 04: Traps (陷阱处理)

**目标**: 掌握底层硬件交互

* 分析 RISC-V 汇编代码和调用约定
* 实现 `backtrace` 内核调用栈追踪
* 构建用户级定时器中断机制

**技术深度**:

* 理解硬件中断和软件陷阱
* 掌握栈帧结构和函数调用
* 实现异步事件处理

### Lab 05: Copy-on-Write Fork (写时复制)

**目标**: 优化进程创建性能

* 实现 Copy-on-Write 机制
* 引用计数管理物理页面
* 处理页错误和动态分配

**性能提升**: 将 fork 操作从 O(n) 优化到 O(1), 显著提升系统性能.

### Lab 06: Networking (网络编程)

**目标**: 构建基础网络栈

* 实现 E1000 网卡驱动
* 构建 ARP, IP, UDP 协议栈
* 提供 socket 接口给用户程序

**核心挑战**:

* 实现 DMA 数据传输, 内存管理
* 处理并发访问和同步问题
* 实现网络协议栈

### Lab 07: Locks (锁优化)

**目标**: 解决多核并发瓶颈

* 内存分配器优化: 从全局锁改为 per-CPU 设计, 实现 work-stealing 机制
* 缓冲区缓存优化: 使用 Hash Table 实现 per-bucket 锁, 并应用 CLOCK 算法

**性能成果**:

* 内存分配器: 减少锁竞争, 提高并发性能
* 缓冲区缓存: 全局锁的竞争消除

**工程价值**: 高性能系统设计的核心技术, per-bucket 锁在数据库系统缓冲池管理器中广泛应用.

### Lab 08: File System (文件系统)

**目标**: 扩展文件系统功能

* 大文件支持: 实现双重间接块, 支持最大 65803 blocks (约 270 MB) 的文件
* 符号链接: 实现软链接功能和路径解析

**技术挑战**: 处理复杂的文件系统数据结构和路径解析逻辑.

### Lab 09: MMAP (内存映射)

**目标**: 实现现代内存管理核心功能

* 实现 `mmap` 和 `munmap` 系统调用
* VMA (虚拟内存区域) 管理系统
* 懒加载和按需分页机制

**核心实现**: Virtual Memory Area (VMA) 数据结构

```c
struct vma {
    uint64 start, length;
    int prot, flags;
    struct file *f;
    uint64 offset;
};
```

**技术复杂度**:

* 处理并发页错误
* 实现部分解映射
* 管理文件引用计数

**实际意义**: 这是现代应用程序 (如数据库, 编译器) 的核心依赖技术.

## 项目进展过程

* **基础层** (Lab 0-2): 环境搭建 → Unix编程 → 系统调用
* **核心层** (Lab 3-5): 内存管理 → 中断处理 → 性能优化
* **高级层** (Lab 6-9): 网络栈 → 并发控制 → 文件系统 → 高级内存管理
* **学习曲线**: 从简单的用户程序到复杂的内核子系统, 循序渐进地掌握操作系统的各个核心组件.

## 项目总结

### 项目规模与成果

* **项目用时**: 约 90 小时
* **测试通过率**: 100% (所有 Lab 通过官方 autograder 测试)

### 核心技术收获

**系统编程能力**:

* 熟练掌握 C 语言内核级编程
* 理解 RISC-V 汇编和硬件接口
* 掌握 GDB 内核调试技术

**操作系统核心概念**:

* 虚拟内存管理: 页表, MMU, 地址翻译
* 进程管理: 调度, 同步, IPC
* 文件系统: inode, 目录, 缓存
* 网络栈: 设备驱动, 协议实现

**性能优化技能**:

* 并发控制: 锁设计, 竞态条件处理
* 内存优化: Copy-on-Write, 懒加载
* 系统调用优化: 减少用户-内核切换开销

### 心得体会

通过这个项目, 不仅掌握了操作系统的理论知识, 更重要的是获得了系统级编程的实践经验, 为未来的技术发展奠定了坚实基础.
