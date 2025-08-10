#pragma once

#include "fiber.hpp"

#include <queue>

class Scheduler {
 public:
  // provide fiber with scheduler stack pointer,
  // and add fiber into run queue
  void add_to_queue(Fiber* fb);

  // execute threads in the run queue, until all are complete
  void run();

 private:
  std::queue<Fiber*> _run_queue;
  void* _stack_ptr{nullptr};
};
