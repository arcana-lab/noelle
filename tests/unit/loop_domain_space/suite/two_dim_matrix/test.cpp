#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int64_t iterations = 10 * std::fmax(argc, 3);
  int64_t iterations2 = (iterations / 2);
  int64_t arr2[iterations][iterations2];

  for (int64_t i = 0; i < iterations; ++i) {
    for (int64_t j = 0; j < iterations2; ++j) {
      arr2[i][j] = i * (i + iterations);
      if (i > 5 || j < 50) {
        arr2[i][j] += i;
      }
    }
  }

  printf("%ld, %ld\n", arr2[0][0], arr2[iterations][iterations2 / 2]);

  return 0;
}
