#ifndef CPPLIBS_UTIL_FORMAT_HPP
#define CPPLIBS_UTIL_FORMAT_HPP

#include <stdexcept>
#include <string>
#include <vector>
#include "util/obj2str.hpp"

// Lightweight C++17 string formatting using {} placeholders.
// Mirrors the std::format interface from C++20.
//
// Supported placeholders:
//   {}    — format next argument using str()
//   {{    — literal '{'
//   }}    — literal '}'
//
// Example:
//   fmt::format("hello {}, value = {}", name, 42)
//   fmt::format("coords ({}, {})", x, y)
//   fmt::format("{{not a placeholder}}")  →  "{not a placeholder}"

namespace fmt {

namespace detail {

// Collect all arguments as strings up front.
inline void collect(std::vector<std::string> &) {}

template<typename T, typename... Rest>
inline void collect(std::vector<std::string> &out, const T &head, const Rest &...tail) {
  out.push_back(str(head));
  collect(out, tail...);
}

inline std::string apply(const std::string &fmt_str, const std::vector<std::string> &args) {
  std::string result;
  result.reserve(fmt_str.size() + args.size() * 8);

  std::size_t arg_idx = 0;
  std::size_t i = 0;

  while (i < fmt_str.size()) {
    char c = fmt_str[i];

    if (c == '{') {
      if (i + 1 < fmt_str.size() && fmt_str[i + 1] == '{') {
        // Escaped '{{'
        result += '{';
        i += 2;
      } else if (i + 1 < fmt_str.size() && fmt_str[i + 1] == '}') {
        // Placeholder '{}'
        if (arg_idx >= args.size()) {
          throw std::runtime_error(
              "fmt::format: too few arguments for format string");
        }
        result += args[arg_idx++];
        i += 2;
      } else {
        throw std::runtime_error(
            "fmt::format: invalid placeholder (use '{}' or '{{' to escape)");
      }
    } else if (c == '}') {
      if (i + 1 < fmt_str.size() && fmt_str[i + 1] == '}') {
        // Escaped '}}'
        result += '}';
        i += 2;
      } else {
        throw std::runtime_error(
            "fmt::format: stray '}' (use '}}' to escape)");
      }
    } else {
      result += c;
      ++i;
    }
  }

  return result;
}

} // namespace detail

// Format a string with {} placeholders.
// Each {} is replaced by the next argument converted via str().
template<typename... Args>
inline std::string format(const std::string &fmt_str, const Args &...args) {
  std::vector<std::string> strs;
  strs.reserve(sizeof...(args));
  detail::collect(strs, args...);
  return detail::apply(fmt_str, strs);
}

// Overload for string literal fmt_str.
template<typename... Args>
inline std::string format(const char *fmt_str, const Args &...args) {
  return format(std::string(fmt_str), args...);
}

} // namespace fmt

#endif // CPPLIBS_UTIL_FORMAT_HPP
