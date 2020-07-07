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

  long long int *oneElement = (long long int *) calloc(0, sizeof(long long int) * 1);

  int v1;
  v1 = argc;
  for (uint32_t i = 0; i < iterations; ++i) {

    for (uint64_t j = i; j < iterations; ++j) {
      auto val = *oneElement;
      v1 += val + i;
      v1 -= 2 + j;
      (*oneElement) = val + v1;
    }

    if (i % 10000 == 0) {
      (*oneElement) = iterations;
    }

  }

  printf("%d, %lld\n", v1, *oneElement);
  return 0;
}
