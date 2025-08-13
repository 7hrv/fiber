#include "context.hpp"
#include "fiber_direct.hpp"
#include "fiber_schedule.hpp"
#include "scheduler.hpp"

#include <chrono>
#include <iostream>
#include <pthread.h>
#include <ratio>
#include <sched.h>
#include <semaphore>
#include <thread>

constexpr int ITERATIONS{1'000'000};

void test_regular_threads() {
  std::binary_semaphore sem1{1};
  std::binary_semaphore sem2{0};

  std::jthread one{[&] {
    for (int i{}; i < ITERATIONS; ++i) {
      sem1.acquire();
      sem2.release();
    }
  }};

  std::jthread two{[&] {
    for (int i{}; i < ITERATIONS; ++i) {
      sem2.acquire();
      sem1.release();
    }
  }};

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);

  pthread_setaffinity_np(one.native_handle(), sizeof(cpu_set_t), &cpuset);
  pthread_setaffinity_np(two.native_handle(), sizeof(cpu_set_t), &cpuset);
}

void test_fibers() {
  Scheduler sched;

  FiberSchedule one{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  FiberSchedule two{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  sched.load_stack_ptr_and_add_to_queue(&one);
  sched.load_stack_ptr_and_add_to_queue(&two);

  sched.run();
}

void test_fiber_direct_yield() {
  FiberDirect one{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  FiberDirect two{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  one.set_peer(&two);
  two.set_peer(&one);

  uint8_t* main_stack_ptr{nullptr};

  one.set_exit_stack_ptr(&main_stack_ptr);
  two.set_exit_stack_ptr(&main_stack_ptr);

  Fiber::set_current_fiber(&one);

  context_switch(static_cast<void*>(&main_stack_ptr), *one.get_stack_ptr_addr());
}

int main() {
  auto start_threads{std::chrono::high_resolution_clock::now()};
  test_regular_threads();
  auto end_threads{std::chrono::high_resolution_clock::now()};

  std::chrono::duration<double, std::milli> duration_threads{end_threads - start_threads};
  std::cout << "Execution time for regular threads: " << duration_threads.count() << " ms\n";

  auto start_fibers{std::chrono::high_resolution_clock::now()};
  test_fibers();
  auto end_fibers{std::chrono::high_resolution_clock::now()};

  std::chrono::duration<double, std::milli> duration_fibers{end_fibers - start_fibers};
  std::cout << "Execution time for regular fibers: " << duration_fibers.count() << " ms\n";

  auto start_fibers_no_sched{std::chrono::high_resolution_clock::now()};
  test_fiber_direct_yield();
  auto end_fibers_no_sched{std::chrono::high_resolution_clock::now()};

  std::chrono::duration<double, std::milli> duration_fibers_direct{end_fibers_no_sched -
                                                                   start_fibers_no_sched};
  std::cout << "Execution time for fiber direct yield " << duration_fibers_direct << " ms\n";

  return 0;
}
