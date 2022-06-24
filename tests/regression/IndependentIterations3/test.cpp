#include <math.h>
#include <stdio.h>
#include <stdlib.h>

long long int computeSum(long long int *a, long long int iters) {
  long long int s = 1;
  long long int s2 = 0;

  for (auto i = 0; i < iters; ++i) {
    s += a[i];

    s += i;

    s += a[i];

    s2 *= s;
  }

  return s + s2;
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
  long long int *array =
      (long long int *)malloc(sizeof(long long int) * iterations);

  for (auto i = 0; i < iterations; i++) {
    array[i] = i;
  }

  auto s = computeSum(array, iterations);
  printf("%lld\n", s);

  return 0;
}
