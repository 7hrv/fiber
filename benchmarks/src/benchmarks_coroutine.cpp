#include "benchmarks.hpp"
#include "benchmarks_common.hpp"

#include <coroutine>
#include <cstdint>
#include <exception>
#include <vector>

namespace {

constexpr int K_BATCH{1024};  // K_ITERS must be a multiple of this

// Empty asm: no instructions emitted, but the compiler can no longer
// prove which coroutine this handle refers to (GCC/Clang only).
inline std::coroutine_handle<> opaque(std::coroutine_handle<> h) {
  void* p{h.address()};
  asm volatile("" : "+r"(p));
  return std::coroutine_handle<>::from_address(p);
}

struct Task {
  struct promise_type {
    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
  };

  explicit Task(std::coroutine_handle<promise_type> h) : handle{h} {}
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  ~Task() {
    if (handle) {
      handle.destroy();
    }
  }

  std::coroutine_handle<promise_type> handle;
};

struct TransferTo {
  const std::coroutine_handle<>* target;

  [[nodiscard]] bool await_ready() const noexcept { return false; }

  [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept {
    return opaque(*target);
  }

  void await_resume() const noexcept {}
};

Task worker(int iters, const std::coroutine_handle<>* peer, std::vector<double>* samples) {
  int64_t prev{bench::now_ns()};
  for (int i{}; i < iters; ++i) {
    co_await TransferTo{peer};
    if (samples && (i + 1) % K_BATCH == 0) {
      const int64_t t{bench::now_ns()};
      // one iteration = two transfers (this -> peer -> this)
      (*samples)[(i + 1) / K_BATCH - 1] = double(t - prev) / (2.0 * K_BATCH);
      prev = t;
    }
  }
}

int64_t run_pingpong(int iters, std::vector<double>* samples) {
  std::coroutine_handle<> a_handle;
  std::coroutine_handle<> b_handle;

  Task a{worker(iters, &b_handle, samples)};
  Task b{worker(iters, &a_handle, nullptr)};
  a_handle = a.handle;
  b_handle = b.handle;

  const int64_t start{bench::now_ns()};
  a_handle.resume();
  return bench::now_ns() - start;
}

}  // namespace

namespace bench {
void bench_coroutines() {
  Report rep;
  rep.name = "coroutine, symmetric transfer";
  rep.clock_ns = clock_overhead_ns();

  run_pingpong(K_ITERS,
               nullptr);  // warm-up; pin the thread before this, as in the other benchmarks

  const Usage before{usage_now()};
  for (int r{}; r < K_RUNS; ++r) {
    rep.run_ns.push_back(double(run_pingpong(K_ITERS, nullptr)) / (2.0 * K_ITERS));
  }
  rep.usage = usage_now() - before;
  rep.handoffs = double(K_RUNS) * 2.0 * K_ITERS;

  rep.sample_ns.resize(K_ITERS / K_BATCH);
  run_pingpong(K_ITERS, &rep.sample_ns);  // already ns per transfer; no clock subtraction needed

  print(rep);
}
}  // namespace bench
