# cpplibs

A collection of lightweight, header-only C++17 utilities.

## Requirements

- C++17 or later
- CMake 3.16+

## Installation

Copy the `include/` directory into your project, or add this repo as a subdirectory:

```cmake
add_subdirectory(cpplibs)
target_link_libraries(your_target PRIVATE cpplibs)
```

---

## Modules

### `bio/binary_io.hpp` — Binary File I/O

RAII binary file reader/writer with support for common STL types and user-defined types via ADL.

**Supported types out of the box:**
- Trivially copyable types (`int`, `double`, structs, ...)
- `std::string`
- `std::vector<T>`
- `std::array<T, N>`
- `std::map<K, V>`, `std::unordered_map<K, V>`
- `std::pair<T1, T2>`, `std::tuple<Ts...>`

**User-defined types** — implement `save` / `load` free functions in the same namespace:

```cpp
#include "bio/binary_io.hpp"

struct Foo {
  int id{};
  std::string name;
  std::vector<float> values;
};

void save(bio::BinaryFile &f, const Foo &x) {
  f.save(x.id);
  f.save(x.name);
  f.save(x.values);
}

void load(bio::BinaryFile &f, Foo &x) {
  f.load(x.id);
  f.load(x.name);
  f.load(x.values);
}

int main() {
  Foo out{7, "demo", {1.5f, 2.5f, 3.5f}};

  // Write
  {
    bio::BinaryFile file("data.bin", bio::BinaryFile::Mode::Write);
    file.save(out);
  }

  // Read
  Foo in{};
  {
    bio::BinaryFile file("data.bin", bio::BinaryFile::Mode::Read);
    file.load(in);
  }
}
```

---

### `util/obj2str.hpp` — Object to String

Converts any common type or container to a human-readable string via `str()`.

```cpp
#include "util/obj2str.hpp"

str(42)                                          // "42"
str(3.14)                                        // "3.140000"
str('A')                                         // "A"
str("hello")                                     // "hello"
str(std::make_pair(1, std::string("one")))        // "1:one"

str(std::vector<int>{1, 2, 3})                   // "[1,2,3]"
str(std::array<int, 3>{1, 2, 3})                 // "[1,2,3]"
str(std::set<int>{1, 2, 3})                      // "{1,2,3}"
str(std::map<std::string, int>{{"a",1},{"b",2}}) // "{a:1,b:2}"

// Nesting
str(std::vector<std::vector<int>>{{1,2},{3,4}})  // "[[1,2],[3,4]]"
str(std::map<std::string, std::vector<int>>{
      {"evens", {2,4}}, {"odds", {1,3}}})         // "{evens:[2,4],odds:[1,3]}"
```

Sequence containers use `[...]`, associative containers use `{...}`.

---

### `util/timer.hpp` — Timing

```cpp
#include "util/timer.hpp"

double t0 = wall_time();   // high-resolution wall clock (seconds)
double c0 = cpu_time();    // CPU time (seconds)
std::string ts = now();    // "2026-04-08 21:00:00"

// ... do work ...
double elapsed = wall_time() - t0;
```

---

### `util/format.hpp` — String Formatting

Lightweight C++17 `{}` placeholder formatting, mirrors `std::format` from C++20.

```cpp
#include "util/format.hpp"

fmt::format("hello {}", name)                // "hello world"
fmt::format("({}, {})", 1.5, 2.5)           // "(1.500000, 2.500000)"
fmt::format("loaded {} items", n)           // "loaded 42 items"
fmt::format("set = {{1, 2, 3}}")            // "set = {1, 2, 3}"  (escaped braces)
fmt::format("map = {}", str(my_map))        // "map = {a:1,b:2}"
```

Each `{}` is replaced by the next argument converted via `str()`. Use `{{`/`}}` for literal braces.

---

### `util/log.hpp` — Logging

Level-gated logging macros with `{}` placeholder formatting. Set `LOG_LEVEL` before including (default: `LOG_INFO`).

```cpp
#define LOG_LEVEL LOG_VERBOSE  // optional, before include
#include "util/log.hpp"

FLOG("fatal: {}", msg);          // always prints to stderr, then exits
ELOG("error: {} at line {}", e, line); // prints at ERROR and above
ILOG("loaded {} items", n);      // prints at INFO and above
VLOG("detail: x={}", x);        // prints at VERBOSE and above
DLOG("trace: ptr={}", ptr);     // prints at DEBUG only

assertf(x > 0, "x must be positive, got {}", x);
log_fatal("unrecoverable: {}", reason);
```

Output format: `[2026-04-08 21:00:00 main.cpp:42] info:loaded 10 items`

Log levels: `LOG_NONE=0` `LOG_ERROR=1` `LOG_INFO=2` `LOG_VERBOSE=3` `LOG_DEBUG=4`

---

### `util/random.hpp` — Random Number Generation

```cpp
#include "util/random.hpp"

Random<int> rng(1, 100);  // uniform int in [1, 100]
rng.seedr();              // seed from hardware random

int x = rng();            // random int in [1, 100]
int y = rng(10, 20);      // random int in [10, 20]

Random<double> drng(0.0, 1.0);
double d = drng();        // random double in [0.0, 1.0]

// Pick a random element from a container
std::vector<int> v = {1, 2, 3, 4, 5};
auto it = select_randomly(v, rng);
```

---

### `util/cmdline.hpp` — Command Line Parsing

```cpp
#include "util/cmdline.hpp"

// ./app --output result.txt --count 5
int main(int argc, char **argv) {
  std::string output;
  int count;

  cmd_opt_set(argc, argv, "--output", output, std::string("out.txt"));
  cmd_opt_set(argc, argv, "--count",  count,  1);

  bool verbose = cmd_has_option(argc, argv, "--verbose");

  // Or with macros (argc/argv must be in scope):
  CMDOPTSET("--output", output, std::string("out.txt"));
  CMDOPTFIND("--verbose");
}
```

---

### `util/debug.hpp` — Debug Printing

```cpp
#include "util/debug.hpp"

int x = 42;
std::string name = "cpplibs";
std::vector<int> nums = {1, 2, 3};

PP_PR(x, name, nums);
// → x = 42, name = cpplibs, nums = [1,2,3],

vlog(x, name);
// → [2026-04-08 21:00:00 main.cpp:10] x=42,name=cpplibs,
```

---

## Building the examples

```bash
cmake -S . -B build
cmake --build build
./build/bio_example
./build/obj2str_example
./build/util_example
```
