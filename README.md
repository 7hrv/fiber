# Fiber

A hand-rolled fiber scheduler with an x86_64 context switch, benchmarked against
`std::thread`, `Boost.Context`, and `C++20 coroutines`. The main purpose of this
project is to learn how `Boost.Context` works as well as learn about the cost of switching between kernel and user mode.

- [overview][./docs/overview.md]
