#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

  auto array = (long long int *) malloc(sizeof(long long int) * iterations);

  array[0] = 42;
  for (auto i=1; i < iterations; i++){
    array[i] = array[i-1] * i;
  }

  int64_t s=0;
  for (auto i=0; i < iterations; i++){
    s += array[i];
  }

  printf("%lld\n", s);

  return 0;
}
