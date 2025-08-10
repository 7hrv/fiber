#include "fiber.hpp"

#include <cstdint>
#include <new>
#include <utility>

namespace {

template <typename T, typename... Args>
void alloc(uint8_t*& current, uint8_t* end, Args&&... args) {
  std::size_t bytes =
      sizeof(T) + (reinterpret_cast<std::uintptr_t>(current) - sizeof(T)) % alignof(T);

  if (current < end) {
    throw std::bad_alloc();  // TODO: this will leak the stack if it fails need to address later
  }

  current -= bytes;
  new (current) T(std::forward<Args>(args)...);
}

}  // namespace

Fiber::Fiber(Fiber::EntryFn entry) : _call_stack{::operator new(Fiber::STACK_SIZE)}, _entry{entry} {
  uint8_t* call_stack_start = static_cast<uint8_t*>(_call_stack);
  _end_of_call_stack = call_stack_start + Fiber::STACK_SIZE;
  _stack_ptr = _end_of_call_stack;  // start at the end since the stack grows downward

  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);  // alloc fake return address
  alloc<std::uintptr_t>(
      _stack_ptr,
      call_stack_start,
      reinterpret_cast<std::uintptr_t>(Fiber::execute));  // alloc return address for trampoline
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc rbp
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc rbx
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc r12
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc r13
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc r14
  alloc<uint64_t>(_stack_ptr, call_stack_start, 0);       // alloc r15
}
