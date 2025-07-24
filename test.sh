#!/usr/bin/env bash

test_branch() {
    local branch=$1
    git checkout "$branch" || { echo "Branch $branch not found!"; exit 1; }
    make clean > /dev/null 2>&1
    make > /dev/null && ./grade-lab-"$branch" -v
    echo "Grading $branch completed."
}

cd Xv6-RISCV
make clean > /dev/null 2>&1
git checkout main

test_branch "util"
test_branch "syscall"
test_branch "pgtbl"
test_branch "traps"
test_branch "cow"
test_branch "net"
test_branch "lock"
test_branch "fs"
test_branch "mmap"
