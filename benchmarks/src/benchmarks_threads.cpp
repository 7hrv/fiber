#include "benchmarks.hpp"
#include "benchmarks_common.hpp"

#include <cstdint>
#include <pthread.h>
#include <sched.h>
#include <semaphore>
#include <thread>
#include <vector>

namespace {

void pin_to_core(int core) {
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(core, &set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

int64_t run_pingpong(int iters, int warmup, int core, std::vector<double>* samples) {
  std::binary_semaphore to_b{0};
  std::binary_semaphore to_a{0};

  std::thread b([&] {
    pin_to_core(core);
    for (int i{}; i < warmup + iters; ++i) {
      to_b.acquire();
      to_a.release();
    }
  });
  pin_to_core(core);

  for (int i{}; i < warmup; ++i) {
    to_b.release();
    to_a.acquire();
  }

  const int64_t start{bench::now_ns()};
  if (samples) {
    int64_t prev{start};
    for (int i{}; i < iters; ++i) {
      to_b.release();
      to_a.acquire();
      const int64_t t{bench::now_ns()};
      (*samples)[i] = double(t - prev);
      prev = t;
    }
  } else {
    for (int i{}; i < iters; ++i) {
      to_b.release();
      to_a.acquire();
    }
  }

  const int64_t total{bench::now_ns() - start};

  b.join();
  return total;
}
}  // namespace

namespace bench {
void bench_threads() {
  bench::Report rep;
  rep.name = "std::thread + binary_semaphore, pinned to one core";
  rep.clock_ns = bench::clock_overhead_ns();

  const bench::Usage before{bench::usage_now()};
  for (int r{}; r < K_RUNS; ++r) {
    const int64_t total{run_pingpong(K_ITERS, K_WARMUP, K_CORE, nullptr)};
    rep.run_ns.push_back(double(total) / (2.0 * K_ITERS));
  }
  rep.usage = bench::usage_now() - before;
  rep.handoffs = double(K_RUNS) * 2.0 * (K_ITERS + K_WARMUP);

  std::vector<double> rtt(K_ITERS);
  run_pingpong(K_ITERS, K_WARMUP, K_CORE, &rtt);
  for (double x : rtt) {
    rep.sample_ns.push_back(std::max(0.0, (x - rep.clock_ns) / 2.0));
  }

  bench::print(rep);
}

};  // namespace bench
