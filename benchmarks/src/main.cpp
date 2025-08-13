#include "benchmarks.hpp"

int main() {
  bench::bench_threads();

  bench::bench_fibers();

  bench::bench_fibers_direct_yield();

  bench::bench_boost_context();

  bench::bench_coroutines();

  return 0;
}
