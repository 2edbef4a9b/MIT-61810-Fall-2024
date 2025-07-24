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
