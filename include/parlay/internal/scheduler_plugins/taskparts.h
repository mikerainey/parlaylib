#ifndef PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#define PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#if defined(PARLAY_TASKPARTS)

// clang++ -I../include -std=c++17 -DPARLAY_TASKPARTS -fno-stack-protector -fno-asynchronous-unwind-tables -fomit-frame-pointer -DTASKPARTS_X64 -DTASKPARTS_POSIX word_counts.cpp -o wc

#include <taskparts/taskparts.hpp>
#include <algorithm>

namespace parlay {

// IWYU pragma: private, include "../../parallel.h"

inline size_t num_workers() { return taskparts::get_nb_workers(); }
inline size_t worker_id() { return taskparts::get_my_id(); }

template <typename Lf, typename Rf>
inline void par_do(Lf&& left, Rf&& right, bool) {
  static_assert(std::is_invocable_v<Lf&&>);
  static_assert(std::is_invocable_v<Rf&&>);
  taskparts::fork2join<Lf, Rf>(std::forward<Lf>(left), std::forward<Rf>(right));
}

template <typename F>
size_t get_granularity(size_t start, size_t end, F f) {
  static_assert(std::is_invocable_v<F&, size_t>);
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

size_t override_granularity = 0;

template <typename F>
void parallel_for_rec(size_t start, size_t end, F&& f, long granularity) {
  if ((end - start) <= granularity) {
    for (size_t i = start; i < end; i++) {
      f(i);
    }
  } else {
    size_t n = end-start;
    size_t mid = (start + (9*(n+1))/16);
    par_do([&] { parallel_for_rec(start, mid, f, granularity); },
	   [&] { parallel_for_rec(mid, end, f, granularity); }, false);
  }
}

template <typename F>
inline void parallel_for(size_t start, size_t end, F&& f, long granularity, bool) {
  static_assert(std::is_invocable_v<F&, size_t>);
  if (end <= start) return;
  granularity = override_granularity == 0 ? granularity : override_granularity;
  if (granularity == 0) {
    size_t done = get_granularity(start, end, f);
    granularity = std::max(done, (end - start) / (128 * num_workers()));
    parallel_for_rec(start + done, end, f, granularity);
  } else {
    parallel_for_rec(start, end, f, granularity);
  }
}

}  // namespace parlay

#endif
#endif  // PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_

