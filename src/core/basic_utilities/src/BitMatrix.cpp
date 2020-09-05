#include "BitMatrix.hpp"

namespace llvm {

void BitMatrix::resize(uint32_t n) {
  N = n;
  bv.clear();
  bv.resize(n * n);
}

uint32_t BitMatrix::idx(uint32_t row, uint32_t col) const {
  assert(row < N);
  assert(col < N);
  return row * N + col;
}

uint32_t BitMatrix::count() const { return bv.count(); }

void BitMatrix::set(uint32_t row, uint32_t col, bool v) {
  const uint32_t i = idx(row, col);

  if (v) {
    bv.set(i);
  } else {
    bv.reset(i);
  }
}

bool BitMatrix::test(uint32_t row, uint32_t col) const {
  const uint32_t i = idx(row, col);

  return bv.test(i);
}

int32_t BitMatrix::firstSuccessor(uint32_t row) const {
  const uint32_t rowBegin = N * row;
  int32_t next = -1;

  if (0 == rowBegin) {
    next = bv.find_first();
  } else {
    next = bv.find_next(rowBegin - 1);
  }

  if (-1 == next) {
    return -1;
  }

  const uint32_t rowEnd = rowBegin + N;
  if (((uint32_t)next) >= rowEnd) {
    return -1;
  }

  return next - rowBegin;
}

int32_t BitMatrix::nextSuccessor(uint32_t row, uint32_t prev) const {
  int32_t next = bv.find_next(N * row + prev);
  if (-1 == next) {
    return -1;
  }

  const uint32_t rowBegin = N * row;
  const uint32_t rowEnd = rowBegin + N;
  if (((uint32_t)next) >= rowEnd) {
    return -1;
  }

  return next - rowBegin;
}

void BitMatrix::transitiveClosure() {
  uint32_t nAdd = 0;

  typedef std::list<uint32_t> Worklist;
  Worklist worklist;
  for (uint32_t i = 0; i < N; ++i) {
    worklist.push_back(i);
  }

  while (!worklist.empty()) {
    uint32_t i = worklist.front();
    worklist.pop_front();

    bool changedI = false;

    // (i->j)
    for (int32_t j = firstSuccessor(i); j != -1; j = nextSuccessor(i, j)) {
      // bit-wise or row[i] |= row[j]

      // (j->k)
      for (int32_t k = firstSuccessor(j); k != -1; k = nextSuccessor(j, k)) {
        // but not (i->k)
        if (!test(i, k)) {
          changedI = true;
          ++nAdd;
          set(i, k);
        }
      }
    }

    if (changedI) {
      for (uint32_t p = 0; p < N; ++p) {
        if (test(p, i)) {
          if (std::find(worklist.begin(), worklist.end(), p) ==
              worklist.end()) {
            worklist.push_back(p);
          }
        }
      }
    }
  }
}

void BitMatrix::dump(raw_ostream &fout) const {
  for (uint32_t row = 0; row < N; ++row) {
    for (uint32_t col = 0; col < N; ++col) {
      if (test(row, col)) {
        fout << '#';
      } else {
        fout << '.';
      }
    }
    fout << '\n';
  }
}

} // namespace llvm
