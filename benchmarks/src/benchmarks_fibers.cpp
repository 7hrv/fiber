#include "benchmarks.hpp"
#include "benchmarks_common.hpp"
#include "context.hpp"
#include "fiber_direct.hpp"
#include "fiber_schedule.hpp"
#include "scheduler.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace {

int64_t run_pingpong_scheduler(int iters, std::vector<double>* samples) {
  Scheduler sched;

  FiberSchedule one{[iters, samples](Fiber* fb) {
    int64_t prev{bench::now_ns()};
    for (int i{}; i < iters; ++i) {
      fb->yield();
      if (samples) {
        const int64_t t{bench::now_ns()};
        (*samples)[i] = double(t - prev);
        prev = t;
      }
    }
  }};

  FiberSchedule two{[iters](Fiber* fb) {
    for (int i{}; i < iters; ++i) {
      fb->yield();
    }
  }};

  sched.load_stack_ptr_and_add_to_queue(&one);
  sched.load_stack_ptr_and_add_to_queue(&two);

  const int64_t start{bench::now_ns()};
  sched.run();
  const int64_t total{bench::now_ns() - start};

  return total;
}

int64_t run_pingpong_direct(int iters, std::vector<double>* samples) {
  FiberDirect one{[iters, samples](Fiber* fb) {
    int64_t prev{bench::now_ns()};
    for (int i{}; i < iters; ++i) {
      fb->yield();
      if (samples) {
        const int64_t t{bench::now_ns()};
        (*samples)[i] = double(t - prev);
        prev = t;
      }
    }
  }};

  FiberDirect two{[iters, samples](Fiber* fb) {
    for (int i{}; i < iters; ++i) {
      fb->yield();
    }
  }};

  one.set_peer(&two);
  two.set_peer(&one);

  uint8_t* main_stack_ptr{nullptr};
  one.set_exit_stack_ptr(&main_stack_ptr);
  two.set_exit_stack_ptr(&main_stack_ptr);

  Fiber::set_current_fiber(&one);

  const int64_t start{bench::now_ns()};
  context_switch(static_cast<void*>(&main_stack_ptr), *one.get_stack_ptr_addr());
  const int64_t total{bench::now_ns() - start};

  return total;
}

}  // namespace

namespace bench {
void bench_fibers() {
  bench::Report rep;
  rep.name = "fibers w/ scheduler";
  rep.clock_ns = bench::clock_overhead_ns();

  const bench::Usage before{bench::usage_now()};
  for (int r{}; r < K_RUNS; ++r) {
    const int64_t total{run_pingpong_scheduler(K_ITERS, nullptr)};
    rep.run_ns.push_back(double(total) / (2.0 * K_ITERS));
  }
  rep.usage = bench::usage_now() - before;
  rep.handoffs = double(K_RUNS) * 2.0 * (K_ITERS);

  std::vector<double> rtt(K_ITERS);
  run_pingpong_scheduler(K_ITERS, &rtt);
  for (double x : rtt) {
    rep.sample_ns.push_back(std::max(0.0, (x - rep.clock_ns) / 2.0));
  }

  bench::print(rep);
}

void bench_fibers_direct_yield() {
  bench::Report rep;
  rep.name = "fibers w/ direct yield";
  rep.clock_ns = bench::clock_overhead_ns();

  const bench::Usage before{bench::usage_now()};
  for (int r{}; r < K_RUNS; ++r) {
    const int64_t total{run_pingpong_direct(K_ITERS, nullptr)};
    rep.run_ns.push_back(double(total) / (2.0 * K_ITERS));
  }
  rep.usage = bench::usage_now() - before;
  rep.handoffs = double(K_RUNS) * 2.0 * K_ITERS;

  std::vector<double> rtt(K_ITERS);
  run_pingpong_direct(K_ITERS, &rtt);
  for (double x : rtt) {
    rep.sample_ns.push_back(std::max(0.0, (x - rep.clock_ns) / 2.0));
  }

  bench::print(rep);
}
};  // namespace bench
