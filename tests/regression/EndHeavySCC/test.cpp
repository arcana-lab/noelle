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

  int someValue = 7;
  int accumValue = -15;
  for (auto i = 0; i < iterations; ++i) {
    bool isEven = someValue % 2 == 0;
    if (isEven) {
      someValue = someValue - 5;
    } else {
      someValue = someValue * 2;
    }

    for (auto j = 0; j < iterations; ++j) {
      accumValue += (someValue - j)/2;
    }
  }

  printf("%d %d\n", someValue, accumValue);
  return 0;
}
