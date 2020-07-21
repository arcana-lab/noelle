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

  float tinySCCValue = 7;
  for (auto i = 0; i < iterations; ++i) {
    int accumValue = sqrt(i);
    for (auto j = 0; j < iterations * iterations; ++j) {
      accumValue += (sqrt(i + j) - j)/2;
    }

    bool isEven = accumValue % 2 == 0;
    if (isEven) {
      tinySCCValue += 1;
    } else {
      tinySCCValue *= 0.9;
    }
  }

  printf("%.2f\n", tinySCCValue);
  return 0;
}
