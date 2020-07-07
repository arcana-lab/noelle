#define DEBUG_TYPE "bitmatrix"

#include "../include/BitMatrix.h"
#include "llvm/Support/Debug.h"
#include <vector>
#include <list>

namespace llvm
{

void BitMatrix::resize(unsigned n)
{
  N = n;
  bv.clear();
  bv.resize(n*n);
}

BitMatrix::~BitMatrix()
{
  LLVM_DEBUG(errs() << "BitMatrix " << bv.count() << " / " <<  N*N << " full.\n");
}

unsigned BitMatrix::idx(unsigned row, unsigned col) const
{
  assert( row < N );
  assert( col < N );
  return row * N + col;
}

unsigned BitMatrix::count() const
{
  return bv.count();
}

void BitMatrix::set(unsigned row, unsigned col, bool v)
{
  const unsigned i = idx(row,col);

  if( v )
    bv.set(i);
  else
    bv.reset(i);
}

bool BitMatrix::test(unsigned row, unsigned col) const
{
  const unsigned i = idx(row,col);

  return bv.test(i);
}

int BitMatrix::firstSuccessor(unsigned row) const
{
  const unsigned row_begin = N*row;
  int next = -1;

  if( 0 == row_begin )
    next = bv.find_first();
  else
    next = bv.find_next(row_begin-1);

  if( -1 == next )
    return -1;

  const unsigned row_end = row_begin + N;
  if( ((unsigned)next) >= row_end )
    return -1;

  return next - row_begin;
}

int BitMatrix::nextSuccessor(unsigned row, unsigned prev) const
{
  int next = bv.find_next( N*row + prev );
  if( -1 == next )
    return -1;

  const unsigned row_begin = N*row;
  const unsigned row_end = row_begin + N;
  if( ((unsigned)next) >= row_end )
    return -1;

  return next - row_begin;
}

void BitMatrix::transitive_closure()
{
  unsigned nAdd=0;

  typedef std::list<unsigned> Worklist;
  Worklist worklist;
  for(unsigned i=0; i<N; ++i)
    worklist.push_back(i);

  while( !worklist.empty() )
  {
    unsigned i = worklist.front();
    worklist.pop_front();

    bool changed_i = false;

    // (i->j)
    for(int j=firstSuccessor(i); j != -1; j=nextSuccessor(i,j))
    {
      // bit-wise or row[i] |= row[j]

      // (j->k)
      for(int k=firstSuccessor(j); k != -1; k=nextSuccessor(j,k))
        // but not (i->k)
        if( ! test(i,k) )
        {
          changed_i = true;
          ++nAdd;
          set(i,k);
        }
    }

    if( changed_i )
      for(unsigned p=0; p<N; ++p)
        if( test(p,i) )
          if( std::find(worklist.begin(),worklist.end(),p) == worklist.end() )
            worklist.push_back(p);
  }

  //LLVM_DEBUG(errs() << "Trans closure added " << nAdd << " edges\n");
}

void BitMatrix::dump(raw_ostream &fout) const
{
  for(unsigned row=0; row<N; ++row)
  {
    for(unsigned col=0; col<N; ++col)
    {
      if( test(row,col) )
        fout << '#';
      else
        fout << '.';
    }
    fout << '\n';
  }
}

}
