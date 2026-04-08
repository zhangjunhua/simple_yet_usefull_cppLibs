#ifndef CPPLIBS_UTIL_LOG_HPP
#define CPPLIBS_UTIL_LOG_HPP

#include <cstdio>
#include <cstdlib>
#include <string>
#include "util/timer.hpp"

// Log levels: none < error < info < verbose < debug
#define LOG_NONE    0
#define LOG_ERROR   1
#define LOG_INFO    2
#define LOG_VERBOSE 3
#define LOG_DEBUG   4

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

namespace log_detail {
// Extract filename from a full path.
inline std::string get_filename(const std::string &path) {
  size_t pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos + 1);
}
} // namespace log_detail

// Internal: raw log without newline.
#define LOG_RAW_N(format, ...) \
  printf("[%s %s:%d] " format, now().c_str(), log_detail::get_filename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__)

// Internal: raw log with newline.
#define LOG_RAW(format, ...) LOG_RAW_N(format "\n", ##__VA_ARGS__)

// Fatal log: prints to stderr and exits.
#define log_fatal(format, ...) \
  { \
    fprintf(stderr, "[%s %s:%d] fatal:" format "\n", now().c_str(), log_detail::get_filename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
  }

// Assert with formatted error message.
#define assertf(cond, fmt, ...) if (!(cond)) log_fatal(fmt, ##__VA_ARGS__)

// Level-gated logs.
#define FLOG(format, ...) log_fatal(format, ##__VA_ARGS__)
#define ELOG(format, ...) if constexpr (LOG_LEVEL >= LOG_ERROR)   LOG_RAW("error:"   format, ##__VA_ARGS__)
#define ILOG(format, ...) if constexpr (LOG_LEVEL >= LOG_INFO)    LOG_RAW("info:"    format, ##__VA_ARGS__)
#define VLOG(format, ...) if constexpr (LOG_LEVEL >= LOG_VERBOSE) LOG_RAW("verbose:" format, ##__VA_ARGS__)
#define DLOG(format, ...) if constexpr (LOG_LEVEL >= LOG_DEBUG)   LOG_RAW("debug:"   format, ##__VA_ARGS__)

#endif // CPPLIBS_UTIL_LOG_HPP
