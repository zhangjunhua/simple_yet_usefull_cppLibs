#ifndef CPPLIBS_UTIL_TIMER_HPP
#define CPPLIBS_UTIL_TIMER_HPP

#include <ctime>
#include <string>

// CPU time in seconds since program start.
[[maybe_unused]] inline double cpu_time() {
  return clock() / static_cast<double>(CLOCKS_PER_SEC);
}

// Wall-clock time in seconds (high resolution).
inline double wall_time() {
  timespec ts{};
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec + ts.tv_nsec / 1.0e9;
}

// Current datetime as "YYYY-MM-DD HH:MM:SS".
inline std::string now() {
  time_t t = time(nullptr);
  char tmp[64];
  strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
  return {tmp};
}

#endif // CPPLIBS_UTIL_TIMER_HPP
