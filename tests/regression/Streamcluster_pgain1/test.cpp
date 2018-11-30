#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static bool * is_valid;
static int * values;

int pgain1 (int iters){

  /*
   * Mimics the first loop in the function pgain in streamcluster
   */
  int count = 0;
  for (auto i=0; i < iters; ++i){
    if (is_valid[i]) {
      values[i] = count++;
    }
  }

  return count;
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
  if (iterations < 1) return -1;

  is_valid = (bool *)calloc(iterations, sizeof(bool));
  values = (int *)malloc(iterations * sizeof(int));

  /*
  iterations = 3;
  bool is_valid[3] = { false, false, false };
  int values[3] = { 1, 2, 3 };
  */

  for (auto i=0; i < iterations; i++){
    if (i % 5 < 2) {
      is_valid[i] = 1;
    }
  }

  /*
  int count = 4;
  for (auto i=0; i < iterations; ++i){
    if (is_valid[i]) {
      values[i] = count;
    }
  }
  */
  auto count = pgain1(iterations);

  printf("%d, %d\n", count, values[iterations/2]);

  return 0;
}
