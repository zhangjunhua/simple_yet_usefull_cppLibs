#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

struct Point {
  int x{};
  int y{};
};

int main() {
  std::variant<int, double, std::string, Point> v;

  // output size of int, double, string, and Point and v in bytes
  std::cout << "sizeof(int) = " << sizeof(int) << "\n";
  std::cout << "sizeof(double) = " << sizeof(double) << "\n";
  std::cout << "sizeof(string) = " << sizeof(std::string) << "\n";
  std::cout << "sizeof(Point) = " << sizeof(Point) << "\n";
  std::cout << "sizeof(variant) = " << sizeof(v) << "\n";

  std::cout << "Choose type: 0=int 1=double 2=string 3=point\n";
  int type = 0;
  if (!(std::cin >> type)) {
    std::cerr << "invalid input\n";
    return 1;
  }

  if (type == 0) {
    int x{};
    std::cin >> x;
    v = x;
  } else if (type == 1) {
    double d{};
    std::cin >> d;
    v = d;
  } else if (type == 2) {
    std::string s;
    std::cin >> s;
    v = s;
  } else if (type == 3) {
    Point p{};
    std::cin >> p.x >> p.y;
    v = p;
  } else {
    std::cerr << "unknown type\n";
    return 1;
  }

  std::visit(
      [](const auto &val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
          std::cout << "int doubled = " << (val * 2) << "\n";
        } else if constexpr (std::is_same_v<T, double>) {
          std::cout << "double with 2 decimals = " << std::fixed << std::setprecision(2)
                    << val << "\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
          std::cout << "string length = " << val.size() << "\n";
        } else if constexpr (std::is_same_v<T, Point>) {
          std::cout << "point magnitude^2 = " << (val.x * val.x + val.y * val.y) << "\n";
        }
      },
      v);

    

  return 0;
}
