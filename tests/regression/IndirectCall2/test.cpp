#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <float.h>

#define FUNCS  96

static int bit_shifter(long int x) {
  int i, n;
  
  i = n = 0; 
  n += (int)(x & 1L);

  return n;
}

static int bit_count(long x) {
  int n = 0;
  if (x) {
    n++;
  }
  return(n);
}

static int bitcount(long i) {
  i = ((i & 0xAAAAAAAAL) >>  1) + (i & 0x55555555L);
  i = ((i & 0xCCCCCCCCL) >>  2) + (i & 0x33333333L);
  i = ((i & 0xF0F0F0F0L) >>  4) + (i & 0x0F0F0F0FL);
  i = ((i & 0xFF00FF00L) >>  8) + (i & 0x00FF00FFL);
  i = ((i & 0xFFFF0000L) >> 16) + (i & 0x0000FFFFL);
  return (int)i;
}

int main(int argc, char *argv[]) {
  clock_t start, stop;
  double ct, cmin = DBL_MAX, cmax = 0;
  int i, cminix, cmaxix;
  long j, n, seed;
  static int (*pBitCntFunc[FUNCS])(long) = {
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter,
    bit_count,
    bitcount,
    bit_shifter

  };
  if (argc<2) {
    fprintf(stderr,"Usage: program <iterations>\n");
    exit(-1);
	}
  long long int iterations=atoll(argv[1]);
 
  long *ns = (long *)calloc(iterations, sizeof(long));
  if (ns == NULL){
    fprintf(stderr, "Memory error\n");
    exit(-1);
  }
  for (j=0; j < iterations; j++){
    n = 0;
    for (i = 0; i < FUNCS; i++) {
      seed = 42;
      n += pBitCntFunc[i](seed);
    }
    ns[j] = n;
  }
  printf("Bits: %ld\n", ns[iterations/2]);

  return 0;
}
