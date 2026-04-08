#include "util/timer.hpp"
#include "util/log.hpp"
#include "util/random.hpp"
#include "util/debug.hpp"

#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  // Timer
  double t0 = wall_time();
  std::cout << "started at: " << now() << "\n";

  // Logging
  ILOG("program started");
  DLOG("this won't print at default LOG_INFO level");

  // Random
  Random<int> rng(1, 100);
  rng.seedr();
  std::vector<int> nums = {rng(), rng(), rng(), rng(), rng()};
  std::cout << "random ints: ";
  for (int x : nums) std::cout << x << " ";
  std::cout << "\n";

  Random<double> drng(0.0, 1.0);
  std::cout << "random double: " << drng() << "\n";

  // PP_PR: print variable names and values
  int x = 42;
  std::string name = "cpplibs";
  PP_PR(x, name, nums);

  // vlog
  vlog(x, name);

  double elapsed = wall_time() - t0;
  std::cout << "elapsed: " << elapsed << "s\n";
}
