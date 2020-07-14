#pragma once

#include "SystemHeaders.hpp"

using namespace llvm;

namespace llvm {

// BitMatrix is a NxN bit-matrix that depicts whether a relation R
// holds for a pair with indices (i,j) (i.e., R(i,j) = 0/1)
// BitMatrix is intended for a dense, asymmetric relation R.
struct BitMatrix {
  BitMatrix(uint32_t n = 1) : N(n), bv(n * n) {}

  // Returns the size of BitVector
  uint32_t count() const;

  // Specifies that row is related to col, i.e., R(row,col) = 1
  void set(uint32_t row, uint32_t col, bool v = true);

  // Checks whether row is related to col,
  // i.e., R(row,col) == 1 (R is not symmetric)
  bool test(uint32_t row, uint32_t col) const;

  // Resizes matrix to nxn
  void resize(uint32_t n);

  // Computes the transitive closure.
  // For example, given a adjacency matrix, it converts it to a connectivity
  // matrix, where (i,j) is set if there is a directed path from i to j
  void transitiveClosure();

  // Emits to fout the BitMatrix
  void dump(raw_ostream &fout) const;

private:
  uint32_t N;
  BitVector bv;

  // For a given row returns the first col that is set.
  // Returns -1 if none found.
  int32_t firstSuccessor(uint32_t row) const;

  // For a given row returns the first col after prev (col>prev) that is set.
  // Returns -1 if none found.
  int32_t nextSuccessor(uint32_t row, uint32_t prev) const;

  // Returns the index corresponding to a pair (row.col)
  // i.e., idx = row * N + col
  uint32_t idx(uint32_t row, uint32_t col) const;
};

} // namespace llvm
