#include <iostream>
#include <string>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>

#include "helper/graph_utils.h"

// **************************************************************
// Writes a symmetric R-MAT graph to a file, so that a benchmark can read its
// graph instead of generating it in every run. Every graph example reads the
// file: those that take a symmetric graph and those that take each edge once.
// The graph has n vertices, rounded to a power of two, and about 10n edges,
// each stored once (20n when both directions are counted).
// **************************************************************
using utils = graph_utils<int>;

int main(int argc, char* argv[]) {
  auto usage = "Usage: gen_rmat_graph <n> <filename>";
  if (argc != 3) { std::cout << usage << std::endl; return 1; }
  long n;
  try { n = std::stol(argv[1]); }
  catch (...) { std::cout << usage << std::endl; return 1; }
  parlay::internal::timer t("Time");
  auto G = utils::rmat_symmetric_graph(n, 20*n);
  t.next("generate");
  utils::print_graph_stats(G);
  utils::write_symmetric_graph_to_file(G, argv[2]);
  t.next("write");
  return 0;
}
