#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoi(argv[1]);
  if (iterations < 0) {
    fprintf(stderr, "USAGE: %s requires a position iteration count\n", argv[0]);
    return -1;
  }

  float accumValue = 3004340340.0F;
  for (auto j = 0; j < iterations * iterations; ++j) {
    accumValue += sqrt(j + 100);
  }
  printf("%.0f\n", accumValue);
  return 0;
}
