#include "fiber.hpp"
#include "scheduler.hpp"

#include <iostream>

void functionA(Fiber* fb) {
  for (int i{}; i < 5; i++) {
    std::cout << "A" << i << "\n";
    fb->yield();
  }
}

void functionB(Fiber* fb) {
  for (int i{}; i < 5; i++) {
    std::cout << "B" << i << "\n";
    fb->yield();
  }
}

/*
 *  We should expect:
 *
 *  A0
 *  B0
 *  A1
 *  B1
 *  A2
 *  B2
 *  A3
 *  B3
 *  A4
 *  B4
 *
 */

int main() {
  Scheduler sched;

  Fiber one{functionA};
  Fiber two{functionB};

  sched.add_to_queue(&one);
  sched.add_to_queue(&two);

  sched.run();

  return 0;
}
