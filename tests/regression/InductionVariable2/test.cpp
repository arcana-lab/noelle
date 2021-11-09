#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeSum (long long int *a, long long int iters, long long int seed, long long int innerIters){
  long long int z = (rand() + 1) %20;
  for (long long int i=0; i < iters; i++){
    for (auto k=0; k < innerIters; k++){
      a[i] /= z;
      z++;
      a[i] += z;
    }
  } 
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 4){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SEED INNER_ITERS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) {
    iterations++;
  }
  iterations *= 100;
  auto seed = atoll(argv[2]);
  auto innerIters = atoll(argv[3]);

  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));
  computeSum(array, iterations, seed, innerIters);
  printf("%lld\n", array[iterations/2]);

  return 0;
}
