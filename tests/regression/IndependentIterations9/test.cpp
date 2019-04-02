#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computeSum (long long int *a, float iters, float startValue){
  long long int s =0;

  for (auto i=(long long int)startValue; i < (long long int)iters ; i++){
    a[i] += iters;
  }

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  long long int *array = (long long int *) malloc(sizeof(long long int) * iterations);

  for (auto i=0; i < iterations; i++){
    array[i] = i;
  }

  auto s = computeSum(array, iterations, 0);
  printf("%lld\n", s);

  return 0;
}
