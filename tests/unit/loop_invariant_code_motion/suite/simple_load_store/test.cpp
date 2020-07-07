#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;

  long long int *array = (long long int *) calloc(0, sizeof(long long int) * iterations);
  long long int *oneElement = (long long int *) calloc(0, sizeof(long long int) * 1);

  int v1;
  v1 = argc;
  for (uint32_t i = 0; i < iterations; ++i) {

    // Can be hoisted
    auto val = *oneElement;
    v1 += val;
    v1 -= 2;
    (*oneElement) = val + v1;

    // Can't be hoisted
    array[i] += v1 * 2;
  }

  printf("%d, %lld, %lld\n", v1, *array, *oneElement);
  return 0;
}


