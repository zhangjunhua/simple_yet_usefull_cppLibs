#ifndef LCTD_HEADERS_HEADER_H_
#define LCTD_HEADERS_HEADER_H_
#include <algorithm>
#include <cassert>
#include <ctime>
#include <execinfo.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>
#include <array>


#define LOG_NONE 0
#define LOG_ERROR 1//
#define LOG_INFO 2 //This Level of log should not affect the performance, GENERAL INFO THAT PROGRAM IS RUNNING IN NORMAL
#define LOG_VERBOSE 3//This level is used for general development, MORE INFO FOR INSIGHT OF THE PROGRAM
#define LOG_DEBUG 4  //This level is used for debug the program, COMPREHENSIVE INFO FOR DEBUG

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO//
#endif

//======================================time utilities=====================================
[[maybe_unused]] inline double cpu_time() { return clock() / static_cast<double>(CLOCKS_PER_SEC); }

inline double wall_time() {
  timespec ts{};
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec + ts.tv_nsec / 1.0e9;
}
inline std::string now() {
  time_t t = time(nullptr);
  char tmp[64];
  strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&t));
  return {tmp};
}

#define get_current_time wall_time

//=========================tools for parsing command line=======================
inline char *getCmdOption(int argc, char **argv, const std::string &option) {
  char **itr = std::find(argv, argv + argc, option);
  if (itr != (argv + argc) && ++itr != (argv + argc)) { return *itr; }
  return nullptr;
}

inline bool cmdOptionExists(int argc, char **argv, const std::string &option) {
  return std::find(argv, argv + argc, option) != (argv + argc);
}

template<typename T, typename T2>
inline void cmd_opt_set(int argc, char **argv, const std::string &opt, T &var, const T2 &dft) {
  char **itr = std::find(argv, argv + argc, opt);
  static_assert(std::is_convertible<T2, T>::value, "Cannot convert from T2 to T");
  if (itr != (argv + argc) && ++itr != (argv + argc)) {
    if constexpr (std::is_integral<T>::value) var = std::stoi(*itr);
    else if constexpr (std::is_floating_point<T>::value)
      var = std::stof(*itr);
    else if constexpr (std::is_same<T, std::string>::value)
      var = *itr;
  } else
    var = dft;//or var=T(dft);
}

#define CMTOPTSET_MACROF(opt, var, dft) cmd_opt_set(argc, argv, opt, var, dft)
#define CMTOPTFIND_MACROF(opt) cmdOptionExists(argc, argv, opt)

////=====================tools for logging===================================
//void print_trace(const std::string &msg) {//compile with -rddynamic too see the demangled symbols
//  char **strings;
//  size_t i, size;
//  enum Constexpr { MAX_SIZE = 1024 };
//  void *array[MAX_SIZE];
//  size = backtrace(array, MAX_SIZE);
//  strings = backtrace_symbols(array, size);
//  for (i = 0; i < size; i++) printf("%s. msg:%s\n", strings[i], msg.c_str());
//  puts("");
//  free(strings);
//}

inline std::string getfilename(std::string path) {
  size_t pos = path.rfind('/');
  if (pos == std::string::npos) {
    return path;
  } else {
    return path.substr(pos + 1, path.size() - pos);
  }
}

inline std::string exec(const std::string &cmd) {
  std::string result;
  std::array<char, 128> buffer{};
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) { throw std::runtime_error("popen() failed!"); }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) { result += buffer.data(); }
  return result;
}

std::vector<std::string> get_stacktrace() {//with -g option.
  std::vector<std::string> stacktrace;
  char **strings;
  int i, size;
  const int MAX_SIZE = 1024;
  void *array[MAX_SIZE];
  size = backtrace(array, MAX_SIZE);
  strings = backtrace_symbols(array, size);
  for (i = 0; i < size; i++) {
    int b = 0, c = 0;
    while (strings[i][b] != '(') b++;
    while (strings[i][c] != ')') c++;
    char syscom[1024];
    sprintf(syscom, "addr2line %.*s -e %.*s", c - b - 1, strings[i] + b + 1, b, strings[i]);
    stacktrace.push_back(getfilename(exec(syscom)));
    stacktrace.back().pop_back();
  }
  free(strings);
  return stacktrace;
}

inline void print_stacktrace() {
  auto stacktrace = get_stacktrace();
  for (auto i = 0; i < stacktrace.size(); ++i) { std::cout << "f #" << i << ": " << stacktrace[i] << std::endl; }
}

//message log
#define mlogn(format, ...)                                                                                             \
  printf("[%s %s:%d] " format, now().c_str(), getfilename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__)
#define mlog(format, ...) mlogn(format "\n", ##__VA_ARGS__)

//error log,
#define elog(format, ...)                                                                                              \
  {                                                                                                                    \
    fprintf(stderr, "[%s %s:%d] " format "\n", now().c_str(), getfilename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                                                                                                \
  }

#define assertf(A, fmt, ...)                                                                                           \
  if (!(A)) elog(fmt, ##__VA_ARGS__)

//deprecated, too many dependencies, hard to manage and error-prone
// #define flogn(format, ...)                                                                                             \
//   fmt::print("[{} {}:{}] " format, now().c_str(), getfilename(__FILE__).c_str(), __LINE__, ##__VA_ARGS__)
// #define flog(format, ...) flogn(format "\n", ##__VA_ARGS__)

#define ELOG(format, ...)                                                                                              \
  if constexpr (LOG_LEVEL >= LOG_ERROR) mlog("error:" format, ##__VA_ARGS__)
#define ILOG(format, ...)                                                                                              \
  if constexpr (LOG_LEVEL >= LOG_INFO) mlog("info:" format, ##__VA_ARGS__)
#define VLOG(format, ...)                                                                                              \
  if constexpr (LOG_LEVEL >= LOG_VERBOSE) mlog("verbose:" format, ##__VA_ARGS__)
#define DLOG(format, ...)                                                                                              \
  if constexpr (LOG_LEVEL >= LOG_DEBUG) mlog("debug:" format, ##__VA_ARGS__)

//==================random generators===================
/**
 * generate type T values uniformly in [min,max]
 * @tparam T
 * @tparam URBG
 */
template<typename T = int, typename URBG = std::minstd_rand>
class Random {
  using DT = std::conditional_t<std::is_integral<T>::value, std::uniform_int_distribution<T>,
                                std::uniform_real_distribution<T>>;
  using paramT = typename DT::param_type;

  URBG _rng;
  DT _dist;

 public:
  inline Random() : Random(T(std::is_integral<T>::value ? std::numeric_limits<T>::max() : T(1))) {};
  explicit Random(T max) : Random(T(0), max) {}
  inline Random(T min, T max) : _rng(0), _dist(min, max) {}

  inline void seed(unsigned int s) { _rng.seed(s); }
  [[maybe_unused]] inline void seedr() { seed(std::random_device()()); }

  inline void set(T max) { set(T(0), max); }
  inline void set(T min, T max) { _dist = DT(min, max); }

  inline T min() { return _dist.min(); }
  inline T max() { return _dist.max(); }
  inline T operator()() { return _dist(_rng); }
  inline T operator()(T max) { return operator()(0, max); }
  inline T operator()(T min, T max) { return _dist(_rng, paramT(min, max)); }
};

[[deprecated("Use Random class instead")]] unsigned long rand_long() {
  static std::mt19937_64 gen(0);
  //  ELOG("");
  //  ILOG("");
  //  VLOG("");
  //  DLOG("");
  return gen();
}

template<typename Container, typename RandomGenerator>
inline typename Container::iterator select_randomly(Container &c, RandomGenerator &g) {
  auto it = c.begin();
  std::advance(it, g(0, std::distance(c.begin(), c.end()) - 1));
  return it;
}

#define PP_CONCAT(A, B) PP_CONCAT_IMPL(A, B)
#define PP_CONCAT_IMPL(A, B) A##B

#define PP_GET_N(N, ...) PP_CONCAT(PP_GET_N_, N)(__VA_ARGS__)
#define PP_GET_N_0(_0, ...) _0
#define PP_GET_N_1(_0, _1, ...) _1
#define PP_GET_N_2(_0, _1, _2, ...) _2
#define PP_GET_N_3(_0, _1, _2, _3, ...) _3
#define PP_GET_N_4(_0, _1, _2, _3, _4, ...) _4
#define PP_GET_N_5(_0, _1, _2, _3, _4, _5, ...) _5
#define PP_GET_N_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define PP_GET_N_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define PP_GET_N_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define PP_NARG(...) PP_GET_N(8, ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define PP_DO_EACH_0(what)
#define PP_DO_EACH_1(what, x) what(x) PP_DO_EACH_0(what)
#define PP_DO_EACH_2(what, x, ...) what(x) PP_DO_EACH_1(what, __VA_ARGS__)
#define PP_DO_EACH_3(what, x, ...) what(x) PP_DO_EACH_2(what, __VA_ARGS__)
#define PP_DO_EACH_4(what, x, ...) what(x) PP_DO_EACH_3(what, __VA_ARGS__)
#define PP_DO_EACH_5(what, x, ...) what(x) PP_DO_EACH_4(what, __VA_ARGS__)
#define PP_DO_EACH_6(what, x, ...) what(x) PP_DO_EACH_5(what, __VA_ARGS__)

#define PP_DO_EACH(what, ...) PP_CONCAT(PP_DO_EACH_, PP_NARG(__VA_ARGS__))(what, ##__VA_ARGS__)

#define PP_WATCH(x) << (#x) << " = " << str(x) << ", "
#define PP_PR(...) std::cout PP_DO_EACH(PP_WATCH, ##__VA_ARGS__) << std::endl

#define vlog_format(x) #x "=%s,"
#define vlog_string(x) ,str(x).c_str()

#define vlog(...) printf("[%s %s:%d] " PP_DO_EACH(vlog_format, ##__VA_ARGS__) "\n", now().c_str(), getfilename(__FILE__).c_str(), __LINE__ PP_DO_EACH(vlog_string, ##__VA_ARGS__))

#endif//LCTD_HEADERS_HEADER_H_