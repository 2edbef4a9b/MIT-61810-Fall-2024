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

### Key Learning Outcomes

These labs provided a deep, hands-on understanding of core file system concepts and the engineering challenges involved in extending a kernel's capabilities.

* **Inode and File System Structure**: Gained a comprehensive understanding of the on-disk and in-memory structures of the xv6 file system, including how inodes, direct/indirect block pointers, and directory entries work together to represent files and directories.
* **Advanced File System Features**: Learned the fundamental differences between hard links and symbolic links at the inode level. Implemented the complete lifecycle of symbolic links, from creation (`sys_symlink`) to resolution (`sys_open`).
* **Extending System Limits**: Mastered the technique of adding new layers of indirection (double indirect blocks) to overcome system limitations, significantly increasing the maximum file size and making the file system more practical.
* **Kernel Path Resolution**: Modified the kernel's core path resolution logic (`namei`, `sys_open`) to handle complex cases like nested symbolic links, demonstrating how system calls interact with the underlying file system layers.
* **Resource Management and Atomicity**: Gained critical experience in managing kernel resources (inodes, file descriptors, disk blocks) within transactional boundaries (`begin_op`/`end_op`). This highlighted the importance of ensuring that all resources are correctly allocated and, crucially, deallocated, especially on failure paths.
* **Defensive Programming in the Kernel**: Learned the importance of writing robust kernel code that anticipates and correctly handles edge cases and potential errors, such as circular symlinks, dangling links, and out-of-space conditions during multi-step operations.

### Challenges and Solutions

Implementing these file system features exposed several classic and subtle kernel programming challenges, primarily related to resource management and the robustness of error-handling paths.

* **Correctly Freeing Multi-Level Block Pointers**: Implementing `itrunc` for large files was challenging. A naive implementation could easily leak entire single-indirect blocks or the double-indirect block itself if the deallocation logic was not perfectly hierarchical.
* **Solution**: We designed a careful, multi-level deallocation process in `itrunc`. The code first iterates through the double-indirect block to free all data blocks and their parent single-indirect blocks, before finally freeing the double-indirect block itself. This ensures that the entire block tree is cleanly deallocated.
* **Releasing Locks/Blocks/Inodes After Use**: A common pitfall in kernel development is failing to release resources correctly, leading to deadlocks or resource leaks. For example, if an inode is locked but not released after a symlink resolution fails, it could block other processes indefinitely.
* **Solution**: We ensured that every path through `sys_open` and `sys_symlink` correctly releases any locked inodes or allocated blocks, even in error cases. This was achieved by carefully structuring the code to always reach the `iunlockput` and `brelse` calls, regardless of whether the operation succeeded or failed.
* **Premature Resource Allocation in `sys_open`**: Another critical bug was identified where `filealloc()` and `fdalloc()` were called before the symbolic link resolution loop. If an error occurred during the loop (e.g., too many symlinks, target not found), the subsequent call to `fileclose()` would attempt to `iput()` a `file->ip` pointer that had not yet been assigned, causing a null-pointer dereference and kernel panic.
* **Solution**: The logic in `sys_open` was reordered to follow the principle of late resource allocation. The `file` structure and file descriptor are now allocated only *after* all path resolution, including following symlinks, has successfully completed and a final, valid inode has been locked. This ensures that the cleanup path (`fileclose`) is never called on a partially initialized `file` structure.
* **Handling Circular Symbolic Links**: A naive implementation of symlink following would enter an infinite loop if faced with a circular link (e.g., a -> b, b -> a).
* **Solution**: We implemented a simple but effective loop counter within the `sys_open` resolution logic. If the number of symbolic link traversals exceeds a reasonable limit (e.g., 10), the operation is aborted, and an error is returned to the user, preventing a kernel hang or stack overflow.

### Conclusion

This lab was a deep dive into the practical engineering of a UNIX-like file system. By implementing support for large files and symbolic links, we moved xv6 from a basic teaching tool closer to a functionally recognizable operating system. The challenges encountered, particularly in ensuring robust error handling and preventing subtle race conditions during path resolution, were profound learning experiences. They underscored that in kernel development, correct resource management across all code paths—especially failure paths—is paramount for system stability. The final, functional file system is a testament to the careful, defensive, and structured programming required to build reliable OS components.
