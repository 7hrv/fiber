#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <sys/resource.h>
#include <vector>

namespace bench {
inline std::int64_t now_ns() {
  using namespace std::chrono;
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

inline double clock_overhead_ns(int n = 1'000'000) {
  volatile int64_t sink{};
  int64_t start{now_ns()};
  for (int i{}; i < n; ++i) {
    sink = now_ns();
  }
  return double(now_ns() - start) / n;
}

inline double percentile(const std::vector<double>& v, double p) {
  return v[static_cast<size_t>(p * (v.size() - 1))];
}

struct Usage {
  long voluntary_cs{};
  long involuntary_cs{};
  double user_s{};
  double sys_s{};
};

inline Usage operator-(const Usage& a, const Usage& b) {
  return {.voluntary_cs = a.voluntary_cs - b.voluntary_cs,
          .involuntary_cs = a.involuntary_cs - b.involuntary_cs,
          .user_s = a.user_s - b.user_s,
          .sys_s = a.sys_s - b.sys_s};
}

inline Usage usage_now() {
  rusage r{};
  getrusage(RUSAGE_SELF, &r);
  auto secs{[](timeval t) { return t.tv_sec + t.tv_usec / 1e6; }};
  return {.voluntary_cs = r.ru_nvcsw,
          .involuntary_cs = r.ru_nivcsw,
          .user_s = secs(r.ru_utime),
          .sys_s = secs(r.ru_stime)};
}

struct Report {
  const char* name{};
  std::vector<double> run_ns;
  std::vector<double> sample_ns;
  double handoffs{};
  Usage usage{};
  double clock_ns{};
};

inline void print(Report rp) {
  std::sort(rp.run_ns.begin(), rp.run_ns.end());
  std::sort(rp.sample_ns.begin(), rp.sample_ns.end());

  const double med{percentile(rp.run_ns, 0.5)};
  const double cpu{rp.usage.user_s + rp.usage.sys_s};

  std::printf("== %s ==\n", rp.name);
  std::printf("handoff latency median %.1f ns min %.1f IQR %.1f-%.1f (%zu runs)\n",
              med,
              rp.run_ns.front(),
              percentile(rp.run_ns, 0.25),
              percentile(rp.run_ns, 0.75),
              rp.run_ns.size());
  std::printf("throughput %.2f M handoffs\n", 1e3 / med);
  std::printf("tail (sampled) p50 %.0f p99 %.0f p99.9 %.0f max %.0f ns (%zu samples)\n",
              percentile(rp.sample_ns, 0.5),
              percentile(rp.sample_ns, 0.99),
              percentile(rp.sample_ns, 0.999),
              rp.sample_ns.back(),
              rp.sample_ns.size());
  std::printf("kernel %.3f voluntary cs/handoff %.5f involuntary, %.0f%% of CPU time in kernel\n",
              rp.usage.voluntary_cs / rp.handoffs,
              rp.usage.involuntary_cs / rp.handoffs,
              cpu > 0 ? 100.0 * rp.usage.sys_s / cpu : 0.0);
  std::printf("clock overhead %.1f ns pernow call\n\n", rp.clock_ns);
}

};  // namespace bench
