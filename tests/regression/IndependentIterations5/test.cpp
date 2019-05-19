#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int *array;

long long int computeSum (long long int *a, long long int iters, long long unsigned int inner_iters){
  long long int s =0;

  for (auto i=0; i < iters; ++i){

    double tmp = 43290.32435F;
    for (auto j=0; j < inner_iters; j++){
      tmp = tmp - 2;
    }

    s += tmp;
  }

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS NESTED_LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto inner_iterations = atoll(argv[2]);

  auto s = computeSum(array, iterations, inner_iterations);
  printf("%lld\n", s);

  return 0;
}
