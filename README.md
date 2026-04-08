# cpplibs

A collection of lightweight, header-only C++17 utilities for everyday systems programming tasks — binary I/O, logging, formatting, timing, randomness, and debugging. No dependencies beyond the C++ standard library.

Each module is a single header you can drop into any project.

## Requirements

- C++17 or later
- CMake 3.16+ (only needed to build the examples)

## Installation

Copy the `include/` directory into your project and add it to your include path. That's it.

Or, if you use CMake:

```cmake
add_subdirectory(cpplibs)
target_link_libraries(your_target PRIVATE cpplibs)
```

---

## Modules

### `bio/binary_io.hpp` — Binary File I/O

**Problem:** Serializing C++ data structures to disk is tedious. `fwrite`/`fread` only handle raw bytes — you have to manually track sizes for strings, vectors, and nested containers, and remember to read back in exactly the same order you wrote.

**Solution:** `BinaryFile` handles all of that automatically. It supports all common STL types out of the box. For your own types, just implement a pair of free functions and everything composes.

```cpp
#include "bio/binary_io.hpp"

// Works immediately with: int, double, bool, structs (trivially copyable),
// std::string, std::vector<T>, std::array<T,N>,
// std::map<K,V>, std::unordered_map<K,V>, std::pair<T1,T2>, std::tuple<...>

// Write
{
  bio::BinaryFile f("data.bin", bio::BinaryFile::Mode::Write);
  f.save(42);
  f.save(std::string("hello"));
  f.save(std::vector<float>{1.0f, 2.0f, 3.0f});
}

// Read — must mirror write order
{
  bio::BinaryFile f("data.bin", bio::BinaryFile::Mode::Read);
  int n; f.load(n);
  std::string s; f.load(s);
  std::vector<float> v; f.load(v);
}
```

**User-defined types** — implement `save`/`load` free functions in the same namespace as your type (ADL picks them up automatically):

```cpp
struct Record {
  int id{};
  std::string name;
  std::vector<float> scores;
};

void save(bio::BinaryFile &f, const Record &r) {
  f.save(r.id);
  f.save(r.name);
  f.save(r.scores);
}

void load(bio::BinaryFile &f, Record &r) {
  f.load(r.id);
  f.load(r.name);
  f.load(r.scores);
}

// Now save/load work like any built-in type:
Record rec{1, "alice", {9.5f, 8.0f}};
{
  bio::BinaryFile f("rec.bin", bio::BinaryFile::Mode::Write);
  f.save(rec);
}
Record rec2;
{
  bio::BinaryFile f("rec.bin", bio::BinaryFile::Mode::Read);
  f.load(rec2);
}
```

RAII: the file is flushed and closed when `BinaryFile` goes out of scope.

---

### `util/obj2str.hpp` — Object to String

**Problem:** Printing C++ containers for debugging requires writing loops, or reaching for `std::ostringstream`. Nested containers are even worse.

**Solution:** A single `str()` function that handles any arithmetic type, string, pair, or STL container — including nested ones.

```cpp
#include "util/obj2str.hpp"

str(42)          // "42"
str(3.14)        // "3.140000"
str('A')         // "A"
str("hello")     // "hello"

str(std::make_pair(1, std::string("one")))        // "1:one"
str(std::vector<int>{1, 2, 3})                    // "[1,2,3]"
str(std::array<int, 3>{1, 2, 3})                  // "[1,2,3]"
str(std::set<int>{1, 2, 3})                       // "{1,2,3}"
str(std::map<std::string, int>{{"a",1},{"b",2}})  // "{a:1,b:2}"

// Nesting just works
str(std::vector<std::vector<int>>{{1,2},{3,4}})               // "[[1,2],[3,4]]"
str(std::map<std::string, std::vector<int>>{{"evens",{2,4}}}) // "{evens:[2,4]}"
```

Sequence containers use `[...]`, associative containers use `{...}`, pairs use `first:second`.

---

### `util/format.hpp` — String Formatting

**Problem:** `printf` requires knowing the type of every argument (`%d`, `%s`, `%.2f`, ...) and is unsafe — wrong format specifiers cause undefined behavior. `std::ostringstream` is type-safe but verbose. `std::format` is ideal but requires C++20.

**Solution:** A lightweight `fmt::format` that works in C++17. Uses `{}` placeholders like `std::format`, and delegates type conversion to `str()`, so it automatically handles containers, pairs, and anything else `obj2str.hpp` supports.

```cpp
#include "util/format.hpp"

fmt::format("hello, {}!", name)              // "hello, world!"
fmt::format("loaded {} of {} items", n, total) // "loaded 3 of 10 items"
fmt::format("pos = ({}, {})", x, y)          // "pos = (1.500000, 2.500000)"
fmt::format("data = {}", str(my_vec))        // "data = [1,2,3]"
fmt::format("{{literal braces}}")            // "{literal braces}"
```

Each `{}` is replaced by the next argument. Use `{{` and `}}` to emit literal braces. Throws `std::runtime_error` if there are too few arguments for the placeholders.

---

### `util/log.hpp` — Logging

**Problem:** `printf` debugging is fine until you want log levels, timestamps, and file/line info. Adding all that manually every time is repetitive and easy to forget.

**Solution:** Five macros (`FLOG/ELOG/ILOG/VLOG/DLOG`) that automatically prepend a timestamp and source location, gate output by log level, and use `{}` placeholder formatting — no format specifiers needed.

```cpp
#define LOG_LEVEL LOG_VERBOSE  // set before include; default is LOG_INFO
#include "util/log.hpp"

ILOG("server started on port {}", port);      // prints at INFO and above
ELOG("connection failed: {}", error_msg);     // prints at ERROR and above
VLOG("received {} bytes from {}", n, addr);  // prints at VERBOSE and above
DLOG("internal state: {}", str(state_map));  // prints at DEBUG only
FLOG("unrecoverable error: {}", reason);     // prints to stderr and exits

assertf(ptr != nullptr, "null pointer in {}", __func__);
```

Output: `[2026-04-08 21:00:00 main.cpp:42] info:server started on port 8080`

Control verbosity by setting `LOG_LEVEL` before including the header:

| Macro | Level | When it prints |
|-------|-------|----------------|
| `FLOG` | fatal | always (then exits) |
| `ELOG` | error | `LOG_LEVEL >= LOG_ERROR` |
| `ILOG` | info  | `LOG_LEVEL >= LOG_INFO` (default) |
| `VLOG` | verbose | `LOG_LEVEL >= LOG_VERBOSE` |
| `DLOG` | debug | `LOG_LEVEL >= LOG_DEBUG` |

---

### `util/timer.hpp` — Timing

**Problem:** Measuring how long something takes in C++ requires pulling in `<chrono>` and writing several lines of boilerplate every time.

**Solution:** Three one-liners for the most common timing needs.

```cpp
#include "util/timer.hpp"

double t0 = wall_time();        // wall-clock time in seconds (high resolution)
double c0 = cpu_time();         // CPU time consumed by this process in seconds
std::string ts = now();         // current datetime as "2026-04-08 21:00:00"

// ... do work ...

double elapsed = wall_time() - t0;
```

Use `wall_time()` to measure real elapsed time. Use `cpu_time()` to measure how much CPU the process consumed (useful for multi-threaded work where wall time < CPU time).

---

### `util/random.hpp` — Random Number Generation

**Problem:** The C++11 `<random>` API is correct but verbose: you need to declare an engine, a distribution, and wire them together every time you want a random number.

**Solution:** A `Random<T>` wrapper that holds both and lets you call it like a function.

```cpp
#include "util/random.hpp"

Random<int> rng(1, 100);  // uniform int in [1, 100]
rng.seedr();              // seed from hardware entropy source

int x = rng();            // draw from [1, 100]
int y = rng(10, 20);      // one-off draw from [10, 20], ignores default range

Random<double> drng(0.0, 1.0);
double d = drng();        // uniform double in [0.0, 1.0]

// Pick a uniformly random element from any container
std::vector<int> v = {10, 20, 30, 40, 50};
auto it = select_randomly(v, rng);  // returns an iterator
```

---

### `util/cmdline.hpp` — Command Line Parsing

**Problem:** Parsing `argc`/`argv` by hand is error-prone. Most argument-parsing libraries are heavyweight for simple scripts and tools.

**Solution:** Three functions that cover the common cases: read a named option's value, check if a flag is present, and set a variable with a default fallback.

```cpp
#include "util/cmdline.hpp"

// ./app --output result.txt --count 5 --verbose
int main(int argc, char **argv) {
  std::string output;
  int count;

  // Set variable from named option, with a default if not found
  cmd_opt_set(argc, argv, "--output", output, std::string("out.txt"));
  cmd_opt_set(argc, argv, "--count",  count,  1);

  // Check if a flag is present (boolean)
  bool verbose = cmd_has_option(argc, argv, "--verbose");

  // Get the raw string after an option
  char *raw = cmd_get_option(argc, argv, "--output");  // nullptr if absent

  // Convenience macros when argc/argv are in scope:
  CMDOPTSET("--output", output, std::string("out.txt"));
  bool v = CMDOPTFIND("--verbose");
}
```

`cmd_opt_set` supports `int`, `float`/`double`, and `std::string` targets.

---

### `util/debug.hpp` — Debug Printing

**Problem:** During development you often want to quickly print a variable and see its name alongside its value. `printf` requires typing the name twice and the format specifier manually.

**Solution:** `PP_PR` and `vlog` print variable names and values automatically. They work with any type that `str()` handles, including containers.

```cpp
#include "util/debug.hpp"

int x = 42;
std::string name = "cpplibs";
std::vector<int> nums = {1, 2, 3};

// Print to stdout with variable names
PP_PR(x, name, nums);
// → x = 42, name = cpplibs, nums = [1,2,3],

// Print to stdout with timestamp and source location
vlog(x, name);
// → [2026-04-08 21:00:00 main.cpp:10] x=42,name=cpplibs,
```

`PP_PR` is good for quick inspection. `vlog` is better when you need to know when and where the print happened (e.g. in a loop or callback).

---

## Building the examples

```bash
cmake -S . -B build
cmake --build build
./build/bio_example
./build/obj2str_example
./build/util_example
```
