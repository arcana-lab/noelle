#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computeSum (int *a, long long int iters){
  long long int s =0;

  for (auto i=0; i < iters; ++i){
    s += a[i];
  }

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 4){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SCC1_ITERATIONS SCC2_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  int *array = (int *) malloc(sizeof(int) * iterations);

  for (auto i=0; i < iterations; i++){
    array[i] = i;
  }

  auto s = computeSum(array, iterations);
  printf("%lld\n", s);

  return 0;
}
