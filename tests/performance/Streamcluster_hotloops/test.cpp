#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static bool * is_valid;
static int * values;

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
    values[i] = iterations;
  }

  int total_value = 0;
  for (auto i=0; i < iterations; ++i) {
    bool is_it = is_valid[i];
    for (auto j=0; j < values[i]; ++j) {
      total_value += (i + 3) * (j - 5);
      if (is_it ^ (j < 10)) {
        total_value += i + j;
      }
    }
  }

  printf("%d, %d\n", total_value, values[iterations/2]);

  return 0;
}

