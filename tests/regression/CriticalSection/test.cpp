#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  /*
   * Fetch the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s ITERATIONS\n", argv[0]);
    return 1;
  }
  auto iters = atoll(argv[1]);
  iters *= 10;
  printf("Running %lld iterations\n", iters);

  /*
   * Allocate memory.
   */
  int *m = (int *)malloc(sizeof(int) * 4);
  *m = 0;

  /*
   * Initialize variables.
   */
  auto initial_value = (double) rand();

  #pragma omp parallel for schedule(static, 1)
  for (int i=0; i < iters; i++){
    double v = initial_value;

    if (i > 42){
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
    }

    if (v < 42){
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
      v += sqrt(v);
    }

    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);
    v -= sqrt(v);

    /*
     * Critical section
     */
    #pragma omp critical
    m[i%4] += (int)v;
  }

  printf("Result = %d\n", *m);

  /*
   * Free the memory.
   */
  free(m);

  return 0;
}
