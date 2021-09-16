#ifndef PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#define PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#if defined(PARLAY_TASKPARTS)

#include <taskparts/benchmark.hpp>

namespace parlay {

// IWYU pragma: private, include "../../parallel.h"

inline size_t num_workers() { return taskparts::perworker::nb_workers(); }
inline size_t worker_id() { return taskparts::perworker::my_id(); }

using taskparts_scheduler = minimal_scheduler<bench_stats, bench_logging, bench_elastic>;

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) {
  taskparts::fork2join<taskparts_scheduler>(left, right);
}

template <typename F>
inline void parallel_for(size_t start, size_t end, F f,
                         long granularity,
			 bool) {
  if ((end - start) <= granularity)
    for (size_t i=start; i < end; i++) f(i);
  else {
    size_t n = end-start;
    size_t mid = (start + (9*(n+1))/16);
    par_do([&] {
      parallel_for(start, mid, f, granularity);
    }, [&] {
      parallel_for(mid, end, f, granularity);
    });
  }
}

static
bool taskparts_launched = false;

template <typename Benchmark>
auto benchmark_taskparts(const Benchmark& benchmark) {
  taskparts::benchmark_nativefj([&] (auto sched) {
    taskparts_launched = true;
    benchmark();
  });
}

}  // namespace parlay

#endif
#endif  // PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_

