#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS INNER_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto innerIterations = atoll(argv[2]);
  printf("Outer iterations = %lld\n", iterations);
  printf("Inner iterations = %lld\n", innerIterations);

  int v1, v2, v1_temp;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;
  auto values = (int *) calloc(innerIterations, sizeof(int));

  for (auto i = 0; i < iterations; ++i) {

    // SCC 1
    for (auto iter = 0; iter < innerIterations; iter++){
      values[iter] = (int)sqrt((double)(iter + v2));
    }
    v2 += (i * sqrt(v2));
  }
  printf("%d, %d\n", v2, values[0]);

  return 0;
}
