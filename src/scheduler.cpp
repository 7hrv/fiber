#include "scheduler.hpp"

#include "context.hpp"
#include "fiber_schedule.hpp"

void Scheduler::add_to_queue(FiberSchedule* fb) {
  _run_queue.push(fb);
}

void Scheduler::load_stack_ptr_and_add_to_queue(FiberSchedule* fb) {
  fb->set_scheduler_ptr(&_stack_ptr);
  _run_queue.push(fb);
}

void Scheduler::run() {
  while (not _run_queue.empty()) {
    FiberSchedule* fb{_run_queue.front()};
    _run_queue.pop();

    Fiber::set_current_fiber(fb);

    fb->set_state(Fiber::FiberState::RUNNING);

    context_switch(static_cast<void*>(&_stack_ptr), *fb->get_stack_ptr_addr());

    Fiber::set_current_fiber(nullptr);

    if (fb->get_state() != Fiber::FiberState::DONE) {
      fb->set_state(Fiber::FiberState::WAITING);
      _run_queue.push(fb);
    }
  }
}
