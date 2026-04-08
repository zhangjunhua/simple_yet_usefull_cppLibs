
#ifndef OBJ2STR_H
#define OBJ2STR_H

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

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, char>, std::string>
str(const T &obj) {
  return std::to_string(obj);
}

inline std::string str(char c) {
  return std::string(1, c);
}

inline std::string str(const char *s) {
  return s;
}

inline std::string str(const std::string &obj) {
  return obj;
}

template<typename T1, typename T2>
std::string str(const std::pair<T1, T2> &p) {
  return str(p.first) + ":" + str(p.second);
}

// Sequence containers (vector, deque, list, ...) -> [...]
// Associative containers (set, map, unordered_map, ...) -> {...}
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

#endif //OBJ2STR_H
