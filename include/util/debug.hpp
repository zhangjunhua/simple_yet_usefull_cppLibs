#ifndef CPPLIBS_UTIL_DEBUG_HPP
#define CPPLIBS_UTIL_DEBUG_HPP

#include <iostream>
#include "util/obj2str.hpp"
#include "util/timer.hpp"
#include "util/log.hpp"

// PP_PR: print variables with their names to stdout.
// Usage: PP_PR(x, y, z);  →  x = 1, y = 2.5, z = [1,2,3]
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

#define PP_WATCH_(x) << (#x) << " = " << str(x) << ", "
#define PP_PR(...) std::cout PP_DO_EACH(PP_WATCH_, ##__VA_ARGS__) << std::endl

// vlog: log variable names and values.
// Usage: vlog(x, y);  →  [2026-04-08 12:00:00 main.cpp:10] x=42,y=3.14,
#define vlog_entry_(x) + (#x "=" + str(x) + ",")
#define vlog(...) fputs((log_detail::prefix(__FILE__, __LINE__) \
                         PP_DO_EACH(vlog_entry_, ##__VA_ARGS__) + "\n").c_str(), stdout)

#endif // CPPLIBS_UTIL_DEBUG_HPP
