#include "context.hpp"

#include <cstddef>
#include <new>

class Fiber {
  static constexpr std::size_t STACK_SIZE{64 << 10};

public:
  Fiber() : call_stack_(::operator new(STACK_SIZE)) {}

  ~Fiber() { ::operator delete(call_stack_); }

  Fiber(const Fiber &) = delete;
  Fiber &operator=(const Fiber &) = delete;

  Fiber(Fiber &&fib) noexcept {
    if (this == &fib) {
      return;
    }

    call_stack_ = fib.call_stack_;

    ::operator delete(fib.call_stack_);
  }

  Fiber &operator=(Fiber &&fib) noexcept {
    if (this == &fib) {
      return *this;
    }

    ::operator delete(call_stack_);

    call_stack_ = fib.call_stack_;

    ::operator delete(fib.call_stack_);

    return *this;
  }

  void switch_context(Context *old_context, Context *new_context);

private:
  void *call_stack_;
};
