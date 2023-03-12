#ifndef PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#define PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_
#if defined(PARLAY_TASKPARTS)

// clang++ -I../include -std=c++17 -DPARLAY_TASKPARTS -fno-stack-protector -fno-asynchronous-unwind-tables -fomit-frame-pointer -DTASKPARTS_X64 -DTASKPARTS_POSIX word_counts.cpp -o wc
#include "/home/rainey/Work/website/blog/microtaskparts.cpp"
#include <algorithm>

namespace parlay {

// IWYU pragma: private, include "../../parallel.h"

inline size_t num_workers() { return taskparts::get_nb_workers(); }
inline size_t worker_id() { return taskparts::get_my_id(); }

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) {
  taskparts::fork2join<Lf, Rf>(left, right);
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

size_t override_granularity = 0;
  
template <typename F>
inline void parallel_for(size_t start, size_t end, F f,
                         long granularity,
			 bool conservative) {
  if (end <= start) return;
  granularity = override_granularity == 0 ? granularity : override_granularity;
  if (granularity == 0) {
    size_t done = get_granularity(start, end, f);
    granularity = std::max(done, (end - start) / (128 * num_workers()));
    parfor_(start + done, end, f, granularity, conservative);
  } else
    parfor_(start, end, f, granularity, conservative);
}

}  // namespace parlay

#endif
#endif  // PARLAY_INTERNAL_SCHEDULER_PLUGINS_TASKPARTS_HPP_

