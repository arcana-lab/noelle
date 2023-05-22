#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// This loop is provably commutative.
// Therefore there is no need for HELIX to wait in the noncritical path.

int main(int argc, char *argv[]) {
  int *m = (int *)malloc(sizeof(int) * 4);
  *m = 0;
  int num_args = argc - 1;
  printf("num_args = %d\n", num_args);

  for (long long unsigned int i = 0; i < 50000000L; ++i) {

    // make computation expensive so that the loop is not split
    double v = (double)atoi(argv[1]);
    v += sqrt(v);
    v += sqrt(v);
    v += sqrt(v);
    v += sqrt(v);
    v += sqrt(v);

    if (v < 42) {
      continue;
    } else {
      m[i % 4] += 24;
    }
  }

  printf("array = %d\n", *m);
  free(m);
  return 0;
}
