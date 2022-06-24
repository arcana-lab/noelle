#include <math.h>
#include <stdio.h>
#include <stdlib.h>

long long int computeSum(long long int *a,
                         long long int iters,
                         long long int seed,
                         long long int innerIters) {
  long long int s = 0;

  int j = 0;
  long long int i = 0;
  auto z = (rand() + 1) % 20;
  do {
    printf("Start iteration %lld\n", i);

    for (auto k = 0; k < innerIters; k++) {
      a[i] += j + 2;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] /= z;
      z++;
      a[i] += z;
      a[i] *= 1000;
    }

    printf("End iteration %lld\n", i);

    i++;

  } while (i < iters);

  return s;
}

int main(int argc, char *argv[]) {

  /*
   * Check the inputs.
   */
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SEED INNER_ITERS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0)
    return 0;
  iterations *= 100;
  auto seed = atoll(argv[2]);
  auto innerIters = atoll(argv[3]);

  long long int *array =
      (long long int *)calloc(iterations, sizeof(long long int));
  for (auto i = 0; i < iterations; i++) {
    array[i] = i * 5;
  }

  auto s = computeSum(array, iterations, seed, innerIters);
  long long int t = 0;
  for (auto i = 0; i < iterations; i++) {
    t += array[i];
  }
  printf("%lld %lld %lld\n", s, t, array[iterations / 2]);

  return 0;
}
