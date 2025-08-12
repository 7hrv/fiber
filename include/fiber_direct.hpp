#pragma once

#include "context.hpp"
#include "fiber.hpp"

#include <cstdint>
#include <utility>

class FiberDirect : public Fiber {
 public:
  FiberDirect(EntryFn entry) : Fiber(entry) {}

  ~FiberDirect() override = default;

  FiberDirect(const FiberDirect&) = delete;
  FiberDirect& operator=(const FiberDirect&) = delete;

  FiberDirect(FiberDirect&& fib) noexcept
      : Fiber(std::move(fib)), _peer(fib._peer), _exit_stack_ptr(fib._exit_stack_ptr) {}

  FiberDirect& operator=(FiberDirect&& fib) noexcept {
    if (this == &fib) {
      return *this;
    }

    Fiber::operator=(std::move(fib));
    _peer = fib._peer;
    _exit_stack_ptr = fib._exit_stack_ptr;

    return *this;
  }

  void set_peer(Fiber* peer_fb) { _peer = peer_fb; }

  void set_exit_stack_ptr(uint8_t** exit_stack_ptr) { _exit_stack_ptr = exit_stack_ptr; }

  void yield() override {
    if (_peer->get_state() != FiberState::DONE) {
      _current = _peer;
      context_switch(static_cast<void*>(&_stack_ptr), *_peer->get_stack_ptr_addr());
    } else {
      context_switch(static_cast<void*>(&_stack_ptr), *_exit_stack_ptr);
    }
  }

 private:
  // the exit stack pointer to yield execution to
  // when peer is complete
  uint8_t** _exit_stack_ptr{nullptr};

  // the peer to yield execution to
  Fiber* _peer{nullptr};
};
