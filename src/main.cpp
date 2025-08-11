#include "fiber.hpp"
#include "scheduler.hpp"

#include <chrono>
#include <iostream>
#include <pthread.h>
#include <ratio>
#include <sched.h>
#include <semaphore>
#include <thread>

constexpr int ITERATIONS{10'000'000};

void test_regular_threads() {
  std::binary_semaphore sem1{1};
  std::binary_semaphore sem2{0};

  std::jthread one{[&]() {
    for (int i{}; i < ITERATIONS; ++i) {
      sem1.acquire();
      sem2.release();
    }
  }};

  std::jthread two{[&]() {
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

  Fiber one{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  Fiber two{[](Fiber* fb) {
    for (int i{}; i < ITERATIONS; ++i) {
      fb->yield();
    }
  }};

  sched.add_to_queue(&one);
  sched.add_to_queue(&two);

  sched.run();
}

int main() {
  auto start_threads{std::chrono::high_resolution_clock::now()};
  test_regular_threads();
  auto end_threads{std::chrono::high_resolution_clock::now()};

  std::chrono::duration<double, std::milli> duration_threads{end_threads - start_threads};
  std::cout << "Executaion time for regular threads: " << duration_threads.count() << " ms\n";

  auto start_fibers{std::chrono::high_resolution_clock::now()};
  test_fibers();
  auto end_fibers{std::chrono::high_resolution_clock::now()};

  std::chrono::duration<double, std::milli> duration_fibers{end_fibers - start_fibers};
  std::cout << "Execution time for regular fibers: " << duration_fibers.count() << " ms\n";

  return 0;
}
