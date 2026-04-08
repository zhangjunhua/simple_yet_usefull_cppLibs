#include "util/obj2str.hpp"

#include <array>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

int main() {
  // Arithmetic
  std::cout << str(42) << "\n";
  std::cout << str(3.14) << "\n";
  std::cout << str('A') << "\n";

  // Strings
  std::cout << str("hello") << "\n";
  std::cout << str(std::string("world")) << "\n";

  // Pair
  std::cout << str(std::make_pair(1, std::string("one"))) << "\n";

  // Sequence containers -> [...]
  std::cout << str(std::vector<int>{1, 2, 3}) << "\n";
  std::cout << str(std::array<double, 3>{1.1, 2.2, 3.3}) << "\n";

  // Associative containers -> {...}
  std::cout << str(std::set<int>{3, 1, 2}) << "\n";
  std::cout << str(std::map<std::string, int>{{"a", 1}, {"b", 2}}) << "\n";
  std::cout << str(std::unordered_map<std::string, int>{{"x", 10}}) << "\n";

  // Nested
  std::cout << str(std::vector<std::vector<int>>{{1, 2}, {3, 4}}) << "\n";
  std::cout << str(std::map<std::string, std::vector<int>>{{"evens", {2, 4}}, {"odds", {1, 3}}}) << "\n";
}
