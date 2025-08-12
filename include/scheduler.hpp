#pragma once

#include "fiber_schedule.hpp"

#include <queue>

class Scheduler {
 public:
  // add fiber to run queue
  void add_to_queue(FiberSchedule* fb);

  // load stack ptr reference to fiber and then
  // add it to the run queue
  void load_stack_ptr_and_add_to_queue(FiberSchedule* fb);

  // execute threads in the run queue, until all are complete
  void run();

 private:
  std::queue<FiberSchedule*> _run_queue;
  uint8_t* _stack_ptr{nullptr};
};
