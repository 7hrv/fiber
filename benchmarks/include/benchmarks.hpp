#pragma once

namespace bench {
constexpr int K_ITERS{200'000};
constexpr int K_WARMUP{1'000};
constexpr int K_RUNS{10};
constexpr int K_CORE{0};

void bench_threads();

void bench_fibers();

void bench_fibers_direct_yield();

void bench_boost_context();

void bench_coroutines();
};  // namespace bench
