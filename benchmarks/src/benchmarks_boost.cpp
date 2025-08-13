#include "benchmarks.hpp"
#include "benchmarks_common.hpp"

#include <algorithm>
#include <boost/context/fiber.hpp>
#include <boost/context/fiber_fcontext.hpp>
#include <cstdint>
#include <vector>

namespace {

namespace ctx = boost::context;

int64_t run_pingpong(int iters, std::vector<double>* samples) {
  ctx::fiber one{[iters, samples](ctx::fiber&& caller) {
    int64_t prev{bench::now_ns()};
    for (int i{}; i < iters; ++i) {
      caller = std::move(caller).resume();
      if (samples) {
        const int64_t t{bench::now_ns()};
        (*samples)[i] = double(t - prev);
        prev = t;
      }
    }

    return std::move(caller);
  }};

  const int64_t start{bench::now_ns()};
  for (int i{}; i < iters; ++i) {
    one = std::move(one).resume();
  }

  const int64_t total{bench::now_ns() - start};

  return total;
}

}  // namespace

namespace bench {
void bench_boost_context() {
  bench::Report rep;
  rep.name = "boost context";
  rep.clock_ns = bench::clock_overhead_ns();

  const bench::Usage before{bench::usage_now()};
  for (int r{}; r < K_RUNS; ++r) {
    const int64_t total{run_pingpong(K_ITERS, nullptr)};
    rep.run_ns.push_back(double(total) / (2.0 * K_ITERS));
  }
  rep.usage = bench::usage_now() - before;
  rep.handoffs = double(K_RUNS) * 2.0 * K_ITERS;

  std::vector<double> rtt(K_ITERS);
  run_pingpong(K_ITERS, &rtt);
  for (double x : rtt) {
    rep.sample_ns.push_back(std::max(0.0, (x - rep.clock_ns) / 2.0));
  }

  bench::print(rep);
}
};  // namespace bench
