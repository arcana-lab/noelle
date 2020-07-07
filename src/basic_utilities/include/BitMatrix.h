#ifndef BASIC_UTILITIES_BIT_MATRIX_H
#define BASIC_UTILITIES_BIT_MATRIX_H

#include "llvm/ADT/BitVector.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm
{
using namespace llvm;

// BitMatrix is a NxN bit-matrix that depicts whether a relation R
// holds for a pair with indices (i,j) (i.e., R(i,j) = 0/1)
// BitMatrix is intended for a dense, asymmetric relation R.
struct BitMatrix
{
  BitMatrix(unsigned n = 1) : N(n), bv(n * n) {}

  ~BitMatrix();

  // Returns the size of BitVector
  unsigned count() const;

  // Specifies that row is related to col, i.e., R(row,col) = 1
  void set(unsigned row, unsigned col, bool v = true);

  // Checks whether row is related to col,
  // i.e., R(row,col) == 1 (R is not symmetric)
  bool test(unsigned row, unsigned col) const;

	// Resizes matrix to nxn
  void resize(unsigned n);

	// Computes the transitive closure.
	// For example, given a adjacency matrix, it converts it to a connectivity matrix,
	// where (i,j) is set if there is a directed path from i to j
  void transitive_closure();

  // Emits to fout the BitMatrix
  void dump(raw_ostream &fout) const;

private:
  unsigned N;
  BitVector bv;

  // For a given row returns the first col that is set.
  // Returns -1 if none found.
  int firstSuccessor(unsigned row) const;

  // For a given row returns the first col after prev (col>prev) that is set.
  // Returns -1 if none found.
  int nextSuccessor(unsigned row, unsigned prev) const;

  // Returns the index corresponding to a pair (row.col)
  // i.e., idx = row * N + col
  unsigned idx(unsigned row, unsigned col) const;
};

}
#endif
