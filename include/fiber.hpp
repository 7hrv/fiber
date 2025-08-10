#pragma once

#include "context.hpp"

#include <cstddef>
#include <cstdint>
#include <new>

class Fiber {
  static constexpr std::size_t STACK_SIZE{64 * 1024uz};  // 64 KiB

  using EntryFn = void (*)(Fiber*);

 public:
  enum FiberState : uint8_t {
    READY,
    RUNNING,
    WAITING,
    DONE,
  };

  Fiber(EntryFn entry);

  ~Fiber() { ::operator delete(_call_stack); }

  Fiber(const Fiber&) = delete;
  Fiber& operator=(const Fiber&) = delete;

  Fiber(Fiber&& fib) noexcept {
    if (this == &fib) {
      return;
    }

    _call_stack = fib._call_stack;
    _state = fib._state;
    _stack_ptr = fib._stack_ptr;
    _scheduler_stack_ptr = fib._scheduler_stack_ptr;
    _entry = fib._entry;

    ::operator delete(fib._call_stack);
  }

  Fiber& operator=(Fiber&& fib) noexcept {
    if (this == &fib) {
      return *this;
    }

    ::operator delete(_call_stack);

    _call_stack = fib._call_stack;
    _state = fib._state;
    _stack_ptr = fib._stack_ptr;
    _scheduler_stack_ptr = fib._scheduler_stack_ptr;
    _entry = fib._entry;

    ::operator delete(fib._call_stack);

    return *this;
  }

  FiberState get_state() { return _state; }

  void set_state(FiberState new_state) { _state = new_state; }

  void load_stack_ptr(void*& scheduler_stack_ptr) { _scheduler_stack_ptr = &scheduler_stack_ptr; }

  void* get_stack_ptr() { return _stack_ptr; }

  static void set_current_fiber(Fiber* fb) { _current = fb; }

  // have a fiber yield computation time
  void yield() { context_switch(&_stack_ptr, *_scheduler_stack_ptr); }

  static void execute() {
    _current->_entry(_current);
    _current->set_state(FiberState::DONE);
    _current->yield();
  }

 private:
  static inline thread_local Fiber* _current{nullptr};

  void* _call_stack;
  FiberState _state{READY};
  uint8_t* _stack_ptr{nullptr};  // smoother pointer artihmetic
  uint8_t* _end_of_call_stack{nullptr};
  void** _scheduler_stack_ptr{nullptr};
  EntryFn _entry;
};
