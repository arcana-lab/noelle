#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void computation(long long int *a, long long int iters) {
  int i = iters;
  int j = 0;
  int t = 0;
  int iPrev = 0;
  do {
    t |= j;

    iPrev = i;
    i -= 8;

    j = a[i];

  } while (i <= 0);

  printf("%d %d\n", t, iPrev);
}

int main(int argc, char *argv[]) {

  /*
   * Check the inputs.
   */
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  iterations *= 8;
  long long int *array =
      (long long int *)calloc(iterations, sizeof(long long int));

  computation(array, iterations);

  return 0;
}
