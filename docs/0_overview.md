# Overview

## Summary

Fiber is a small C++ library that runs many tasks on one OS thread by switching stacks
in user space. A context switch saves six registers on the current stack, swaps the
stack pointer, restores six registers from the other stack, and returns. There is no
system call and no kernel scheduler involved.

Preliminary results on my machine (median of 10 runs; see [benchmarks](./benchmarks.md) for caveats)

| Variant                                     | Median | Unit         |
|---------------------------------------------|--------|--------------|
| Coroutine, symmetrics transer (C++20)       | TBD    | per transfer |
| Boost.Context                               | TBD    | per switch   |
| Fiber, direct yield (this project)          | TBD    | per switch   |
| Fiber, through the scheduler (this project) | TBD    | per transfer |
| std::thread + semaphore, pinned to one core | TBD    | per transfer |

## Why Use Fibers?

OS threads are switched by the kernel: a trap into kernel mode, a scheduling decision,
and a return to user mode, with the kernel's work competing with the application for cache. The cost and its variability both matter when many tasks mostly wait and
hand control back and forth.
Fibers move the scheduling decision into the application, which allows for deterministic context switching between fibers, at the cost of cooperative switching and all fibers running on one core per scheduler.
