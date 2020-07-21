#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int iterations = 10 * std::fmax(argc, 3);
  int iterations2 = (iterations / 2);
  int * matrix = (int *)calloc(iterations * iterations2, sizeof(int));

  for (int64_t i = 1; i < iterations - 1; ++i) {
    for (int64_t j = 2; j < iterations2; ++j) {

      /*
       * Index truncation is NOT understood and is conservatively not allowed
       * Index modulo wrapping is NOT understood and is conservatively not allowed
       */
      // Allowed: truncation/sign extension never goes below 32 bits
      int32_t ijIdx = (i * iterations2 + j);
      // Allowed: another form of truncation that never goes below 32 bits
      int64_t ijIdx2 = ((i * iterations2 + j) << 32) >> 32;
      // Not allowed: truncation
      int8_t j8 = (int8_t)j;
      int64_t j64Minus = (int64_t)(j8 - 1);
      int64_t ijTruncate = (i * iterations2 + j64Minus);
      // Not allowed: modular
      int64_t jModulo = (j - 1) % iterations2;
      int64_t ijUnderflowIdx = (i * iterations2 + jModulo);

      matrix[ijIdx] += 7;
      matrix[ijIdx2] += 7;
      matrix[ijTruncate] += 13;
      matrix[ijUnderflowIdx] += 11;
    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * (iterations2 - 1)]);

  return 0;
}
