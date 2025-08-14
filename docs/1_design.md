# Design

## What is a Context

A fiber's context is a single saved stack pointer. Everything else needed to resume
lives on the fiber's own stack at the moment it switches away: the return address
(pushed by the call to the switch function) and the six callee-saved registers (pushed
by the switch function). On x86_64 System V those registers are `rbx`, `rbp`, and `r12`
to `r15`. They are the only registers the calling convention requires a function to
preserve, and from the compiler's point of view a yield is an ordinary function call,
so nothing else needs saving.

## The Context Switch

The switch is a short assembly routine in its own .S file. It is called as a normal
function with two arguments: the address of the slot to save the current stack pointer
into (`rdi`), and the stack pointer value to load (`rsi`).

[The assembly code](../src/context_switch.S)

The context switch is directly written in assembly rather than inline asm in a C++
function because the compiler would otherwise emit its own prologue and epilogue
around the stack swap, and its epilogue would run on the wrong stack. The final
`ret` pops the other stack's return address, which is what moves execution into
the other fiber.

## Starting a New Fiber

A new fiber has never run, so there is no saved state to restore. The constructor
fabricates it by writing the same layout a real switch would have left at the top
of the fiber's stack:

```
top of stack (16-byte aligned)
  top-8    fake return address (0)
  top-16   address of the trampoline    <- popped by the final ret into RIP
  top-24   rbp slot (0)
  top-32   rbx slot (0)
  top-40   r12 slot (0)
  top-48   r13 slot (0)
  top-56   r14 slot (0)
  top 64   r15 slot (0)                 <- initial saved stack pointer
  ...      free space for the fiber's own frames
base of stack
```
```
```

The first switch into the fiber loads that saved pointer, pops six zeros into the
registers, and returns into the trampoline. The switch routine needs no special
case for new fibers. The layout is built from the top downward, every slot is 8
bytes, and the trampoline slot sits on a 16-byte boundary so that after the `ret`
the stack pointer is 8 bytes off a 16-byte boundary, as at a normal function entry.
A misaligned stack typically crasjes on the first SSE instruction, often inside
`iostream`.

## The Trampoline

The trampoline is a static function reached by a `ret`, so it cannot receive arguments.
It finds its fiber through a thread-local pointer to the current fiber, calls the
fiber's entry function, marks the fiber DONE, and switches away for the last time, it
must never return because nothing valid lies below it on the stack.

## Scheduling Modes

### Scheduler

Fibers yield to a central FIFO scheduler, which switches into the next ready fiber.
Each yield costs two switches: fiber to scheduler, then scheduler to the next fiber.
The scheduler owns the current-fiber pointer, re-queues unfinished fibers, and
drops finished ones

### Direct Yield

Two fibers are wired as peers and yield straight to each other one switch per hand-off.
Each yield sets the current-fiber to the target before switching. A fiber also has an
exit slot, the saved stack pointer of whoever started the run, so that a finished fiber
switches back to a valid stack instead of returning into the fake return address.

## Stacks

Each fiber has a fixed 64KiB stack allocated with `operator::new`. There is no guard page, so an overflow silently corrupts the heap neighbor. Pooled stack with guard pages
are listed under future work.
