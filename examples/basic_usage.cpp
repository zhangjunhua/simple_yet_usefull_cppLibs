#include "bio/binary_io.hpp"
#include "obj2str.h"
#include "header.h"


#include <iostream>
#include <string>
#include <vector>

struct Foo {
  int id{};
  std::string name;
  std::vector<float> values;
};

inline void save(bio::BinaryFile &f, const Foo &x) {
  f.save(x.id);
  f.save(x.name);
  f.save(x.values);
}

inline void load(bio::BinaryFile &f, Foo &x) {
  f.load(x.id);
  f.load(x.name);
  f.load(x.values);
}

int main() {
  Foo out{7, "demo", {1.5f, 2.5f, 3.5f}};
  {
    bio::BinaryFile file("examples.bin", bio::BinaryFile::Mode::Write);
    file.save(out);
  }

  Foo in{};
  {
    bio::BinaryFile file("examples.bin", bio::BinaryFile::Mode::Read);
    file.load(in);
  }

  int a=3;
  double b=3.14;
  std::string c="hello";

  vlog(a, b, c);

  std::cout << in.id << " " << in.name << " " << in.values.size() << "\n";
  return 0;
}
