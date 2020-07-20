#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int iterations = 100 * argc;
  int arr[iterations];

  arr[0] = iterations;
  for (auto i = 1; i < iterations - 10; ++i) {
    int derived = i + 2;
    arr[derived] = i * (i + iterations);
    if (derived < iterations / 2) {
      arr[derived] += i;
    }
  }

  printf("%d, %d\n", arr[0], arr[iterations/2]);

  return 0;
}
