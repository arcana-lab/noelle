#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computeSum (long long int *a, long long int iters){
  long long int s =0;

  int j = 0;
  long long int i=0;
  do {

    while (j < 10){
      j += rand();
    }

    a[i] += j;

    i++;

  } while (i < iters);

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
  if (iterations == 0) return 0;

  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));

  auto s = computeSum(array, iterations / 2);
  printf("%lld %lld\n", s, array[iterations/2]);

  return 0;
}
