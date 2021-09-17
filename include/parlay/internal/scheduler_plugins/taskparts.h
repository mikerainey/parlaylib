#ifndef PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#define PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#if defined(PARLAY_TASKPARTS)

#include <taskparts/benchmark.hpp>
#include <algorithm>

namespace parlay {

// IWYU pragma: private, include "../../parallel.h"

bool taskparts_launched = false;

inline size_t num_workers() { return taskparts::perworker::nb_workers(); }
inline size_t worker_id() { return taskparts::perworker::my_id(); }

using taskparts_scheduler = taskparts::minimal_scheduler<taskparts::bench_stats, taskparts::bench_logging, taskparts::bench_elastic>;

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) {
  if (taskparts_launched) {
    taskparts::fork2join<Lf, Rf, taskparts_scheduler>(left, right);
  } else {
    left();
    right();
  }
}

template <typename F>
size_t get_granularity(size_t start, size_t end, F f) {
  size_t done = 0;
  size_t sz = 1;
  int ticks = 0;
  do {
    sz = std::min(sz, end - (start + done));
    auto tstart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < sz; i++) f(start + done + i);
    auto tstop = std::chrono::high_resolution_clock::now();
    ticks = static_cast<int>((tstop - tstart).count());
    done += sz;
    sz *= 2;
  } while (ticks < 1000 && done < (end - start));
  return done;
}

template <typename F>
void parfor_(size_t start, size_t end, F f, size_t granularity,
	     bool conservative) {
  if ((end - start) <= granularity)
    for (size_t i = start; i < end; i++) f(i);
  else {
    size_t n = end - start;
    // Not in middle to avoid clashes on set-associative
    // caches on powers of 2.
    size_t mid = (start + (9 * (n + 1)) / 16);
    par_do([&]() { parfor_(start, mid, f, granularity, conservative); },
	   [&]() { parfor_(mid, end, f, granularity, conservative); },
	   conservative);
  }
}

  
template <typename F>
inline void parallel_for(size_t start, size_t end, F f,
                         long granularity,
			 bool conservative) {
  if (end <= start) return;
  if (granularity == 0) {
    size_t done = get_granularity(start, end, f);
    granularity = std::max(done, (end - start) / (128 * num_workers()));
    parfor_(start + done, end, f, granularity, conservative);
  } else
    parfor_(start, end, f, granularity, conservative);
}

template <typename Benchmark>
auto benchmark_taskparts(const Benchmark& benchmark) {
  taskparts::benchmark_nativeforkjoin([&] (auto sched) {
    taskparts_launched = true;
    benchmark(sched);
  });
}

}  // namespace parlay

#endif
#endif  // PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_

