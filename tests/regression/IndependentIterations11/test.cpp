#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int *array;
unsigned int arraySize;

double hotFunction (long long int iterations, long long int inner_iterations, double (*c) (long long int *a, long long int iters)){
  double s = 1.0f;
  for (auto i=0; i < iterations; i++){
    s += (*c)(array, inner_iterations);
  }

  return s;
}

double computeSum (long long int *a, long long int iters){
  double s = 0.0;

  for (auto i=0; i < iters; ++i){
    s += sqrt((double) (a[i % arraySize] + i + iters));
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

  arraySize = 1024;
  array = (long long int *) calloc(arraySize, sizeof(long long int));
  auto s = hotFunction(iterations, inner_iterations, computeSum);
  printf("%lld\n", (long long int)s);

  free(array);

  return 0;
}
