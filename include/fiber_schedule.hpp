#pragma once

#include "context.hpp"
#include "fiber.hpp"

#include <cstdint>
#include <utility>

class FiberSchedule : public Fiber {
 public:
  explicit FiberSchedule(EntryFn entry) : Fiber(entry) {}

  ~FiberSchedule() override = default;

  FiberSchedule(const FiberSchedule&) = delete;
  FiberSchedule& operator=(const FiberSchedule&) = delete;

  FiberSchedule(FiberSchedule&& fib) noexcept
      : Fiber(std::move(fib)), _scheduler_stack_ptr(fib._scheduler_stack_ptr) {}

  FiberSchedule& operator=(FiberSchedule&& fib) noexcept {
    if (this == &fib) {
      return *this;
    }

    FiberSchedule::operator=(std::move(fib));
    _scheduler_stack_ptr = fib._scheduler_stack_ptr;

    return *this;
  }

  void set_scheduler_ptr(uint8_t** scheduler_ptr) { _scheduler_stack_ptr = scheduler_ptr; }

  void yield() override { context_switch(static_cast<void*>(&_stack_ptr), *_scheduler_stack_ptr); }

 private:
  // the stack pointer of the scheduler to yield execution to
  uint8_t** _scheduler_stack_ptr{nullptr};
};
