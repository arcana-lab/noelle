#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS INITIAL_VALUE\n", argv[0]);
    return -1;
  }
  auto iterations = atoi(argv[1]);
  if (iterations < 0) {
    fprintf(stderr, "USAGE: %s requires a position iteration count\n", argv[0]);
    return -1;
  }
  auto i = atoi(argv[2]);

  int accumValue = i;
  for (auto j = 0; j < iterations * iterations; ++j) {
    accumValue -= sqrt(j);
  }

  printf("%d\n", accumValue);
  return 0;
}
