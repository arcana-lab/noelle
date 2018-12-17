#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static bool * is_valid;
static int * values;

bool * external_global;
static int * alias_global1;
static int * alias_global2;

static int * addressed_global;

int pgain1 (int iters){

  /*
   * Mimics the first loop in the function pgain in streamcluster
   */
  int count = 0;
  for (auto i=0; i < iters; ++i){
    if (is_valid[i]) {
      values[i] = 5;
    }
    count += values[i];
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

  for (auto i=0; i < iterations; i++){
    is_valid[i] = i % 5 < 2;
    values[i] = 1;
  }

  auto count = pgain1(iterations);

  printf("%d, %d\n", count, values[iterations/2]);

  alias_global1 = (int *)calloc(iterations, sizeof(int));
  alias_global2 = alias_global1;
  for (auto i = 1; i < iterations; i++) {
    alias_global1[i-1] = i;
    if (i < 50) {
      alias_global2[i] = alias_global1[i-1];
    } else {
      alias_global2[i] = 0;
    }
  }

  addressed_global = (int *)malloc(iterations * sizeof(int));
  *(addressed_global + 2) = 5;
  printf("%d\n", addressed_global[2]);

  return 0;
}
