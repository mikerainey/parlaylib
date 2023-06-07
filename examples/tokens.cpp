#include <iostream>

#include <parlay/io.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "tokens.h"

// **************************************************************
// Driver
// **************************************************************
int main(int argc, char* argv[]) {
  auto usage = "Usage: tokens <filename>";
  if (argc != 2) std::cout << usage << std::endl;
  else {
    parlay::chars str = parlay::chars_from_file(argv[1]);
    parlay::internal::timer t("Time");
    auto r = tokens(str, [&] (char c) { return c == ' '; });
    t.next("tokens");
    std::cout << "number of space separated tokens: " << r.size() << std::endl;
  }
}
