#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct {
  float *coord;
  unsigned long long int a;
  unsigned long long int c;
  unsigned long long int w;
} Point;

float computeDist (Point p1, Point p2, int dim){
  float result = 0.0;
  for (auto i=0; i < dim; i++){
    auto toAdd = (p1.coord[i] - p2.coord[i]) * (p1.coord[i] - p2.coord[i]);
    result += toAdd;
  }

  return result;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s DIMS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);

  clock_t start, stop;
  double ct, cmin = DBL_MAX, cmax = 0;
  int i, cminix, cmaxix;
  long j, n, seed;
  
  for (i = 0; i < 100; i++) {
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
