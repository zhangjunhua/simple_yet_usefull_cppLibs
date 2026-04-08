#ifndef CPPLIBS_UTIL_OBJ2STR_HPP
#define CPPLIBS_UTIL_OBJ2STR_HPP

#include <string>
#include <type_traits>
#include <utility>

namespace detail {

template<typename C, typename = void>
struct is_associative : std::false_type {};

template<typename C>
struct is_associative<C, std::void_t<typename C::key_type>> : std::true_type {};

template<typename C>
inline constexpr bool is_associative_v = is_associative<C>::value;

template<typename C, typename = void>
struct is_iterable : std::false_type {};

template<typename C>
struct is_iterable<C, std::void_t<decltype(std::declval<C>().begin(),
                                           std::declval<C>().end())>>
    : std::true_type {};

template<typename C>
inline constexpr bool is_iterable_v = is_iterable<C>::value;

} // namespace detail

// Arithmetic types (int, double, ...) -> std::to_string
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, char>, std::string>
str(const T &obj) {
  return std::to_string(obj);
}

// char -> single character string
inline std::string str(char c) { return std::string(1, c); }

// C string
inline std::string str(const char *s) { return s; }

// std::string passthrough
inline std::string str(const std::string &obj) { return obj; }

// Forward declaration for pair (defined after container template).
template<typename T1, typename T2>
std::string str(const std::pair<T1, T2> &p);

// Sequence containers (vector, deque, list, array, ...) -> [a,b,c]
// Associative containers (set, map, unordered_map, ...) -> {a,b,c}
// Supports nesting: vector<vector<int>>, map<string, vector<int>>, etc.
template<typename C>
std::enable_if_t<detail::is_iterable_v<C> && !std::is_same_v<C, std::string>,
                 std::string>
str(const C &container) {
  const char open  = detail::is_associative_v<C> ? '{' : '[';
  const char close = detail::is_associative_v<C> ? '}' : ']';
  std::string s;
  s += open;
  auto it = container.begin();
  if (it != container.end()) {
    s += str(*it++);
    for (; it != container.end(); ++it)
      s += "," + str(*it);
  }
  s += close;
  return s;
}

// std::pair -> "first:second"
template<typename T1, typename T2>
std::string str(const std::pair<T1, T2> &p) {
  return str(p.first) + ":" + str(p.second);
}

#endif // CPPLIBS_UTIL_OBJ2STR_HPP
