#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int iterations = 10 * std::fmax(argc, 3);
  int iterations2 = (iterations / 2);
  int iterations3 = (iterations / 3);
  int * matrix = (int *)calloc(iterations * iterations2 * iterations3, sizeof(int));

  for (int64_t i = 0; i < iterations; ++i) {
    for (int64_t j = 0; j < iterations2; ++j) {

      /*
       * Pointer arithmetic is not understood
       */
      int64_t ijIdx = (i * iterations2 + j) * iterations3;
      int64_t matrixStartPtr = (int64_t)matrix + (ijIdx * sizeof(int));

      for (int64_t k = 0; k < iterations3; ++k) {
        *((int *)(matrixStartPtr + k * sizeof(int))) += i - j;
        *((int *)(matrixStartPtr + k * sizeof(int))) *= 1.005;
      }
    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * iterations2 * (iterations3 / 4)]);

  return 0;
}
