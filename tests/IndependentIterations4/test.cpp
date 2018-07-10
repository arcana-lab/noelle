#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computeSum (long long int *a, long long int iters, long long int subiters){
  long long int s =0;
  long long int s2 =0;

  for (auto i=0; i < iters; ++i){

    // SCC1
    for (auto j=0; j < subiters; j++){
      s += (long long int) sqrt((long long int)a[i]);
    }

    // SCC2
    for (auto j=0; j < subiters; j++){
      s2 += (long long int) sqrt((double)s);
    }
  }

  return s + s2;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SUBLOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto sub_iterations = atoll(argv[2]);
  long long int *array = (long long int *) malloc(sizeof(long long int) * iterations);

  for (auto i=0; i < iterations; i++){
    array[i] = i;
  }

  auto s = computeSum(array, iterations, sub_iterations);
  printf("%lld\n", s);

  return 0;
}
