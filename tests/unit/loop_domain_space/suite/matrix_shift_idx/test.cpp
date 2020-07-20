#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  long long int iterations = 10 * std::fmax(argc, 3);
  long long int iterations2 = (iterations / 2);
  long long int iterations3 = (iterations / 2);
  int * matrix = (int *)calloc(iterations * iterations2 * iterations3, sizeof(int));

  long long int sizeJK = iterations2 * iterations3;
  for (int i = 0; i < iterations; ++i) {
    for (int j = 0; j < iterations2; ++j) {
      for (int k = 0; k < iterations3; ++k) {
        int64_t idx = i * sizeJK + j * iterations3 + k;
        idx = (idx << 32) >> 32;
        matrix[idx] += i - j + k;
      }
    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * iterations2 * iterations3 - 1]);

  return 0;
}
