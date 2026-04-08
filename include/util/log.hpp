#ifndef CPPLIBS_UTIL_LOG_HPP
#define CPPLIBS_UTIL_LOG_HPP

#include <cstdlib>
#include <cstdio>
#include <string>
#include "util/timer.hpp"
#include "util/format.hpp"

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

inline std::string get_filename(const std::string &path) {
  size_t pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

// Build the log prefix: [datetime file:line]
inline std::string prefix(const char *file, int line) {
  return fmt::format("[{} {}:{}] ", now(), get_filename(file), line);
}

} // namespace log_detail

// Internal: emit a fully-formed log line to stdout.
// Usage: LOG_EMIT_("info:", __FILE__, __LINE__, "loaded {} items", n)
#define LOG_EMIT_(level, file, line, fmt_str, ...) \
  fputs((log_detail::prefix(file, line) + level + \
         fmt::format(fmt_str, ##__VA_ARGS__) + "\n").c_str(), stdout)

// Fatal log: prints to stderr and exits.
// Usage: log_fatal("bad value: {}", x)
#define log_fatal(fmt_str, ...) \
  do { \
    fputs((log_detail::prefix(__FILE__, __LINE__) + "fatal:" + \
           fmt::format(fmt_str, ##__VA_ARGS__) + "\n").c_str(), stderr); \
    exit(EXIT_FAILURE); \
  } while(0)

// Assert with formatted error message.
// Usage: assertf(x > 0, "x must be positive, got {}", x)
#define assertf(cond, fmt_str, ...) \
  do { if (!(cond)) log_fatal(fmt_str, ##__VA_ARGS__); } while(0)

// Level-gated logs. Usage: ILOG("loaded {} items", n)
#define FLOG(fmt_str, ...) log_fatal(fmt_str, ##__VA_ARGS__)
#define ELOG(fmt_str, ...) if constexpr (LOG_LEVEL >= LOG_ERROR)   LOG_EMIT_("error:",   __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
#define ILOG(fmt_str, ...) if constexpr (LOG_LEVEL >= LOG_INFO)    LOG_EMIT_("info:",    __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
#define VLOG(fmt_str, ...) if constexpr (LOG_LEVEL >= LOG_VERBOSE) LOG_EMIT_("verbose:", __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)
#define DLOG(fmt_str, ...) if constexpr (LOG_LEVEL >= LOG_DEBUG)   LOG_EMIT_("debug:",   __FILE__, __LINE__, fmt_str, ##__VA_ARGS__)

#endif // CPPLIBS_UTIL_LOG_HPP
