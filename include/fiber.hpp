#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <new>

class Fiber {
 public:
  static constexpr std::size_t STACK_SIZE{64 * 1024uz};  // 64 KiB

  using EntryFn = std::function<void(Fiber*)>;

  enum FiberState : uint8_t {
    READY,
    RUNNING,
    WAITING,
    DONE,
  };

  Fiber(EntryFn entry);

  virtual ~Fiber() { ::operator delete(_call_stack); }

  Fiber(const Fiber&) = delete;
  Fiber& operator=(const Fiber&) = delete;

  Fiber(Fiber&& fib) noexcept
      : _call_stack(fib._call_stack),
        _state(fib._state),
        _stack_ptr(fib._stack_ptr),
        _entry(fib._entry) {
    fib._call_stack = nullptr;
  }

  Fiber& operator=(Fiber&& fib) noexcept {
    if (this == &fib) {
      return *this;
    }

    ::operator delete(_call_stack);

    _call_stack = fib._call_stack;
    _state = fib._state;
    _stack_ptr = fib._stack_ptr;
    _entry = fib._entry;

    fib._call_stack = nullptr;

    return *this;
  }

  FiberState get_state() { return _state; }

  void set_state(FiberState new_state) { _state = new_state; }

  uint8_t** get_stack_ptr_addr() { return &_stack_ptr; }

  static void set_current_fiber(Fiber* fb) { _current = fb; }

  virtual void yield() {}

  static void execute() {
    _current->_entry(_current);
    _current->set_state(FiberState::DONE);
    _current->yield();
  }

 protected:
  static inline thread_local Fiber* _current{nullptr};

  void* _call_stack;
  FiberState _state{READY};
  uint8_t* _stack_ptr{nullptr};  // smoother pointer artihmetic
  uint8_t* _end_of_call_stack{nullptr};
  EntryFn _entry;
};
