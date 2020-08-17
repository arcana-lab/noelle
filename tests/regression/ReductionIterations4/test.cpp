#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s DIMS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations < 1){
    iterations = 1;
  }
  iterations *= 100;

  clock_t start, stop;
  double ct, cmin = DBL_MAX, cmax = 0;
  int i, cminix, cmaxix;
  long j, n, seed;
  
  for (i = 0; i < 10; i++) {
    start = (iterations * i) / 2;
    
    for (j = n = 0, seed = rand(); j < iterations; j++, seed += 13){
	    n += (long)((i + seed/ 42));
    }
    
    stop = start + n * 2;
    ct = (stop - start) / 433449;
    if (ct < cmin) {
     cmin = ct;
     cminix = i;
    }
    if (ct > cmax) {
     cmax = ct;
     cmaxix = i;
    }

    printf("%ds> Time: %7.3f sec.; Bits: %ld\n", i, ct, n);
  }

  printf("\nBest  > %d\n", cminix);
  printf("Worst > %d\n", cmaxix);

  return 0;
}
