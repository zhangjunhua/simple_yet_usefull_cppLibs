#include "util/timer.hpp"
#include "util/log.hpp"
#include "util/format.hpp"
#include "util/random.hpp"
#include "util/debug.hpp"

#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  // Timer
  double t0 = wall_time();
  std::cout << "started at: " << now() << "\n";

  // Logging with {} placeholders (no printf format strings needed)
  ILOG("program started");
  ILOG("pid = {}, level = {}", 42, "info");
  DLOG("this won't print at default LOG_INFO level");

  // fmt::format
  std::string msg = fmt::format("elapsed so far: {} ms", 0);
  std::cout << msg << "\n";

  std::string coords = fmt::format("({}, {})", 1.5, 2.5);
  std::cout << "coords: " << coords << "\n";

  // Escaped braces
  std::cout << fmt::format("set = {{1, 2, 3}}") << "\n";

  // Random
  Random<int> rng(1, 100);
  rng.seedr();
  std::vector<int> nums = {rng(), rng(), rng(), rng(), rng()};
  ILOG("random ints: {}", str(nums));

  Random<double> drng(0.0, 1.0);
  ILOG("random double: {}", drng());

  // PP_PR: print variable names and values
  int x = 42;
  std::string name = "cpplibs";
  PP_PR(x, name, nums);

  // vlog
  vlog(x, name);

  double elapsed = wall_time() - t0;
  ILOG("elapsed: {} s", elapsed);
}
