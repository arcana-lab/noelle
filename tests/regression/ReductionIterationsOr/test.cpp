#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computation (long long int *a, long long int iters){
  long long int s =0;

  for (auto i=0; i < iters; ++i){
    s |=  a[i];
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
  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));

  auto s = computation(array, iterations);
  printf("%lld\n", s);

  return 0;
}
