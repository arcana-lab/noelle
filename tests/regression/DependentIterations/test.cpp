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

  for (auto i=0; i < iterations; i++){
    auto index = (i % (argc - 1)) + 1;
    array[i] = atoll(argv[index]);
  }
  for (auto i=1; i < iterations; i++){
    array[i] = array[i-1] * i;
  }

  int64_t s = array[0] + array[iterations - 1] + array[iterations / 2];
  printf("%lld\n", s);

  return 0;
}
