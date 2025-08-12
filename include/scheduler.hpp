#pragma once

#include "fiber.hpp"

#include <queue>

class Scheduler {
 public:
  // add fiber to run queue
  void add_to_queue(Fiber* fb);

  // load stack ptr reference to fiber and then
  // add it to the run queue
  void load_stack_ptr_and_add_to_queue(Fiber* fb);

  // execute threads in the run queue, until all are complete
  void run();

 private:
  std::queue<Fiber*> _run_queue;
  uint8_t* _stack_ptr{nullptr};
};
