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

  constexpr const char* rule{"------------------------------------------------------------"};

  std::printf("%s\n %s\n%s\n", rule, rp.name, rule);

  std::printf(" Handoff latency (%zu runs)\n", rp.run_ns.size());
  std::printf("   %-16s %9.2f ns\n", "median", med);
  std::printf("   %-16s %9.2f ns\n", "min", rp.run_ns.front());
  std::printf("   %-16s %9.2f - %.2f ns\n",
              "IQR (25-75%)",
              percentile(rp.run_ns, 0.25),
              percentile(rp.run_ns, 0.75));
  std::printf("   %-16s %9.1f M/s\n", "throughput", 1e3 / med);

  if (!rp.sample_ns.empty()) {
    std::printf(" Tail (%zu samples)\n", rp.sample_ns.size());
    std::printf("   %-16s %9.1f ns\n", "p50", percentile(rp.sample_ns, 0.50));
    std::printf("   %-16s %9.1f ns\n", "p99", percentile(rp.sample_ns, 0.99));
    std::printf("   %-16s %9.1f ns\n", "p99.9", percentile(rp.sample_ns, 0.999));
    std::printf("   %-16s %9.1f ns\n", "max", rp.sample_ns.back());
  }

  std::printf(" Kernel\n");
  std::printf("   %-16s %9.5f per handoff\n", "voluntary cs", rp.usage.voluntary_cs / rp.handoffs);
  std::printf(
      "   %-16s %9.5f per handoff\n", "involuntary cs", rp.usage.involuntary_cs / rp.handoffs);
  std::printf("   %-16s %9.1f %% of CPU time\n",
              "kernel time",
              cpu > 0 ? 100.0 * rp.usage.sys_s / cpu : 0.0);

  std::printf(" Clock overhead      %9.1f ns per now() call\n", rp.clock_ns);
  std::printf("%s\n\n", rule);
}

};  // namespace bench
