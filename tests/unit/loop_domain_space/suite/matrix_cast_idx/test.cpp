#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int iterations = 10 * std::fmax(argc, 3);
  int iterations2 = (iterations / 2);
  int * matrix = (int *)calloc(iterations * iterations2, sizeof(int));

  for (int64_t i = 0; i < iterations; ++i) {
    int iIdx = i * iterations2;
    for (int64_t j = 0; j < iterations2; ++j) {
      __int128_t ijIdxBig = i * iterations2 + j;
      matrix[iIdx + j] += i - j + 2;
      matrix[ijIdxBig] -= 5 * (4 - i);
    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * (iterations2 - 1)]);

  return 0;
}
