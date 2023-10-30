#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <sys/resource.h>

namespace parlay {
  namespace internal {

static
struct rusage global_rusage;
static
std::chrono::time_point<std::chrono::steady_clock> global_exectime;

void __attribute__((constructor)) __timer_global_init() {
  getrusage(RUSAGE_SELF, &global_rusage);
  global_exectime = std::chrono::steady_clock::now();
}
    
struct timer {

private: 
  using time_t = decltype(std::chrono::system_clock::now());
  double total_so_far;
  time_t last;
  bool on;
  std::string name;
  std::vector<double> exectimes;
  using rusage_metrics = struct rusage_metrics_struct {
    double utime;
    double stime;
    uint64_t nvcsw;
    uint64_t nivcsw;
    uint64_t maxrss;
    uint64_t nsignals;
  };
  std::vector<rusage_metrics> rusages;
  struct rusage last_rusage;

  auto get_time() {
    return std::chrono::system_clock::now();
  }
  auto double_of_tv(struct timeval tv) {
    return ((double) tv.tv_sec) + ((double) tv.tv_usec)/1000000.;
  }

  void report(double time, std::string str) {
    std::ios::fmtflags cout_settings = std::cout.flags();
    std::cout.precision(4);
    std::cout << std::fixed;
    std::cout << name << ": ";
    if (str.length() > 0)
      std::cout << str << ": ";
    std::cout << time << std::endl;
    std::cout.flags(cout_settings);
  }

  double diff(time_t t1, time_t t2) {
    return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t2).count() / 1000000.0;
  }
public:


  timer(std::string name = "Parlay time", bool start_ = true)
  : total_so_far(0.0), on(false), name(name) {
    if (start_) start();
  }
  void global_output() {
    struct rusage last_rusage;
    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - global_exectime;
    getrusage(RUSAGE_SELF, &last_rusage);
    auto previous_rusage = global_rusage;
    rusage_metrics m = {
      .utime = double_of_tv(last_rusage.ru_utime) - double_of_tv(previous_rusage.ru_utime),
      .stime = double_of_tv(last_rusage.ru_stime) - double_of_tv(previous_rusage.ru_stime),
      .nvcsw = (uint64_t)(last_rusage.ru_nvcsw - previous_rusage.ru_nvcsw),
      .nivcsw = (uint64_t)(last_rusage.ru_nivcsw - previous_rusage.ru_nivcsw),
      .maxrss = (uint64_t)(last_rusage.ru_maxrss),
      .nsignals = (uint64_t)(last_rusage.ru_nsignals - previous_rusage.ru_nsignals)
    };
    std::string outfile = "";
    if (const auto env_p = std::getenv("PARLAYLIB_OVERALL_STATS_OUTFILE")) {
      outfile = std::string(env_p);
    }
    if (outfile == "") {
      return;
    }
    FILE* f = (outfile == "stdout") ? stdout : fopen(outfile.c_str(), "w");
    fprintf(f, "[\n");
    fprintf(f, "{\"exectime\": %f,\n", elapsed.count());
    fprintf(f, "\"usertime\": %f,\n", m.utime);
    fprintf(f, "\"systime\": %f,\n", m.stime);
    fprintf(f, "\"nvcsw\": %lu,\n", m.nvcsw);
    fprintf(f, "\"nivcsw\": %lu,\n", m.nivcsw);
    fprintf(f, "\"maxrss\": %lu,\n", m.maxrss);
    fprintf(f, "\"nsignals\": %lu}", m.nsignals);
    fprintf(f, "\n]\n");
    if (f != stdout) {
      fclose(f);
    }
  }
  ~timer() {
    std::string outfile = "";
    if (const auto env_p = std::getenv("PARLAYLIB_TIMER_OUTFILE")) {
      outfile = std::string(env_p);
    }
    if (outfile == "") {
      return;
    }
    FILE* f = (outfile == "stdout") ? stdout : fopen(outfile.c_str(), "w");
    fprintf(f, "[\n");
    size_t n = exectimes.size();
    for (size_t i = 0; i < n; i++) {
      fprintf(f, "{\"exectime\": %f,\n", exectimes[i]);
      fprintf(f, "\"usertime\": %f,\n", rusages[i].utime);
      fprintf(f, "\"systime\": %f,\n", rusages[i].stime);
      fprintf(f, "\"nvcsw\": %lu,\n", rusages[i].nvcsw);
      fprintf(f, "\"nivcsw\": %lu,\n", rusages[i].nivcsw);
      fprintf(f, "\"maxrss\": %lu,\n", rusages[i].maxrss);
      fprintf(f, "\"nsignals\": %lu}", rusages[i].nsignals);
      if (i + 1 != n) {
	fprintf(f, ",\n");
      }
    }
    fprintf(f, "\n]\n");
    if (f != stdout) {
      fclose(f);
    }
    global_output();    
  }
  
  void start () {
    on = true;
    last = get_time();
    getrusage(RUSAGE_SELF, &last_rusage);
  }

  double stop () {
    on = false;
    double d = diff(get_time(),last);
    total_so_far += d;
    return d;
  }

  void reset() {
     total_so_far=0.0;
     on=0;
  }

  double next_time() {
    if (!on) return 0.0;
    time_t t = get_time();
    double td = diff(t, last);
    total_so_far += td;
    last = t;
    return td;
  }

  double total_time() {
    if (on) return total_so_far + diff(get_time(), last);
    else return total_so_far;
  }

  void next(std::string str) {
    auto t = next_time();
    exectimes.push_back(t);
    auto previous_rusage = last_rusage;
    getrusage(RUSAGE_SELF, &last_rusage);
    rusage_metrics m = {
      .utime = double_of_tv(last_rusage.ru_utime) - double_of_tv(previous_rusage.ru_utime),
      .stime = double_of_tv(last_rusage.ru_stime) - double_of_tv(previous_rusage.ru_stime),
      .nvcsw = (uint64_t)(last_rusage.ru_nvcsw - previous_rusage.ru_nvcsw),
      .nivcsw = (uint64_t)(last_rusage.ru_nivcsw - previous_rusage.ru_nivcsw),
      .maxrss = (uint64_t)(last_rusage.ru_maxrss),
      .nsignals = (uint64_t)(last_rusage.ru_nsignals - previous_rusage.ru_nsignals)
    };
    rusages.push_back(m);
    if (on) report(t, str);
  }

  void total() {
    report(total_time(),"total");
  }
};

  }
}
