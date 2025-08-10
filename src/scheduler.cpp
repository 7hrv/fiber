#include "scheduler.hpp"

#include "context.hpp"
#include "fiber.hpp"

void Scheduler::add_to_queue(Fiber* fb) {
  fb->load_stack_ptr(_stack_ptr);
  _run_queue.push(fb);
}

void Scheduler::run() {
  while (not _run_queue.empty()) {
    Fiber* fb{_run_queue.front()};
    _run_queue.pop();

    Fiber::set_current_fiber(fb);

    fb->set_state(Fiber::FiberState::RUNNING);

    context_switch(&_stack_ptr, fb->get_stack_ptr());

    Fiber::set_current_fiber(nullptr);

    if (fb->get_state() != Fiber::FiberState::DONE) {
      fb->set_state(Fiber::FiberState::WAITING);
      _run_queue.push(fb);
    }
  }
}
