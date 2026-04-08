#ifndef CPPLIBS_UTIL_CMDLINE_HPP
#define CPPLIBS_UTIL_CMDLINE_HPP

#include <algorithm>
#include <string>
#include <type_traits>

// Returns the value after the given option flag, or nullptr if not found.
// Example: --output file.txt  →  getCmdOption(argc, argv, "--output") == "file.txt"
inline char *getCmdOption(int argc, char **argv, const std::string &option) {
  char **itr = std::find(argv, argv + argc, option);
  if (itr != (argv + argc) && ++itr != (argv + argc)) return *itr;
  return nullptr;
}

// Returns true if the given flag exists in argv.
inline bool cmdOptionExists(int argc, char **argv, const std::string &option) {
  return std::find(argv, argv + argc, option) != (argv + argc);
}

// Sets var to the value after opt (parsed to type T), or dft if not found.
// Supports int, float, and std::string.
template<typename T, typename T2>
inline void cmd_opt_set(int argc, char **argv, const std::string &opt, T &var, const T2 &dft) {
  static_assert(std::is_convertible<T2, T>::value, "Cannot convert default value to target type");
  char **itr = std::find(argv, argv + argc, opt);
  if (itr != (argv + argc) && ++itr != (argv + argc)) {
    if constexpr (std::is_integral<T>::value)        var = static_cast<T>(std::stoi(*itr));
    else if constexpr (std::is_floating_point<T>::value) var = static_cast<T>(std::stof(*itr));
    else if constexpr (std::is_same<T, std::string>::value) var = *itr;
  } else {
    var = dft;
  }
}

// Convenience macros (require argc/argv in scope).
#define CMDOPTSET(opt, var, dft) cmd_opt_set(argc, argv, opt, var, dft)
#define CMDOPTFIND(opt)          cmdOptionExists(argc, argv, opt)

#endif // CPPLIBS_UTIL_CMDLINE_HPP
