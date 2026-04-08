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

// Extract filename from a full path.
inline std::string getfilename(std::string path) {
  size_t pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

// Raw message log (no newline).
#define mlogn(format, ...) \
  printf("[%s %s:%d] " format, now().c_str(), getfilename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__)

// Raw message log (with newline).
#define mlog(format, ...) mlogn(format "\n", ##__VA_ARGS__)

// Error log: prints to stderr and exits.
#define elog(format, ...) \
  { \
    fprintf(stderr, "[%s %s:%d] " format "\n", now().c_str(), getfilename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
  }

// Assert with formatted error message.
#define assertf(A, fmt, ...) if (!(A)) elog(fmt, ##__VA_ARGS__)

// Level-gated logs.
#define ELOG(format, ...) if constexpr (LOG_LEVEL >= LOG_ERROR)   mlog("error:"   format, ##__VA_ARGS__)
#define ILOG(format, ...) if constexpr (LOG_LEVEL >= LOG_INFO)    mlog("info:"    format, ##__VA_ARGS__)
#define VLOG(format, ...) if constexpr (LOG_LEVEL >= LOG_VERBOSE) mlog("verbose:" format, ##__VA_ARGS__)
#define DLOG(format, ...) if constexpr (LOG_LEVEL >= LOG_DEBUG)   mlog("debug:"   format, ##__VA_ARGS__)

#endif // CPPLIBS_UTIL_LOG_HPP
