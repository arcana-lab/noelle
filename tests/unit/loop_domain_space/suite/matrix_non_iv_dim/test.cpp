#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int64_t iterations = 10 * std::fmax(argc, 3);
  int64_t iterations2 = (iterations / 2);
  int * matrix = (int *)calloc(iterations * iterations2, sizeof(int));

  for (int64_t i = 0; i < iterations; ++i) {

    /*
     * Constant index is NOT understood yet
     */
    matrix[i * iterations2] = i * (i + iterations);
    matrix[i * iterations2 + 3] = i * (i + iterations);

    for (int64_t j = 5; j < iterations2; ++j) {

      /*
       * Function on sub-loop index is determined to be bounded,
       * and IV derived, so even if it isn't an induction variable itself,
       * it is allowed
       * This is NOT understood yet though
       */
      int boundedI = (int)std::fmin(i, iterations2 - 1);
      matrix[i * iterations2 + boundedI] = i * (i + iterations);
    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * (iterations2 / 2)]);

  return 0;
}
