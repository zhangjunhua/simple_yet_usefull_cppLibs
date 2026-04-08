#ifndef CPPLIBS_UTIL_RANDOM_HPP
#define CPPLIBS_UTIL_RANDOM_HPP

#include <iterator>
#include <limits>
#include <random>
#include <type_traits>

// Generates values of type T uniformly in [min, max].
// Defaults to int with range [0, INT_MAX].
// Usage:
//   Random<int> rng(0, 100);
//   int x = rng();          // random int in [0, 100]
//   int y = rng(10, 20);    // random int in [10, 20]
template<typename T = int, typename URBG = std::minstd_rand>
class Random {
  using DT = std::conditional_t<std::is_integral<T>::value,
                                std::uniform_int_distribution<T>,
                                std::uniform_real_distribution<T>>;
  using paramT = typename DT::param_type;

  URBG _rng;
  DT   _dist;

public:
  Random() : Random(T(0), std::is_integral<T>::value
                            ? std::numeric_limits<T>::max()
                            : T(1)) {}
  explicit Random(T max) : Random(T(0), max) {}
  Random(T min, T max) : _rng(0), _dist(min, max) {}

  void seed(unsigned int s) { _rng.seed(s); }
  void seedr()              { seed(std::random_device()()); }

  void set(T max)         { _dist = DT(T(0), max); }
  void set(T min, T max)  { _dist = DT(min, max); }

  T min() { return _dist.min(); }
  T max() { return _dist.max(); }

  T operator()()            { return _dist(_rng); }
  T operator()(T max)       { return _dist(_rng, paramT(T(0), max)); }
  T operator()(T min, T max){ return _dist(_rng, paramT(min, max)); }
};

// Returns an iterator to a random element in the container.
template<typename Container, typename RandomGenerator>
inline typename Container::iterator select_randomly(Container &c, RandomGenerator &g) {
  auto it = c.begin();
  std::advance(it, g(0, static_cast<typename RandomGenerator::result_type>(
                          std::distance(c.begin(), c.end()) - 1)));
  return it;
}

#endif // CPPLIBS_UTIL_RANDOM_HPP
