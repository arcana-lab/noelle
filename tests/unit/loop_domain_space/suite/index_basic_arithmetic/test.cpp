#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
  int oneOrTwo = argc % 2 == 0 ? 1 : 2;
  int64_t iterations = 30 * oneOrTwo;
  int64_t iterations2 = iterations / 2;
  int * matrix = (int *)calloc(iterations * iterations2, sizeof(int));

  for (int64_t i = 1; i < iterations - 1; ++i) {
    for (int64_t j = 0; j < iterations2; ++j) {

      /*
       * Index arithmetic is understood, and overflowing is not allowed
       */
      // Allowed
      int64_t ijIdx = (i * iterations2 + j);
      // Not allowed: passes over into the next "row" of i
      int64_t ijOverflowIdx = (i * iterations2 + (j + 1));
      // Not allowed: passes over into the previous "row" of i
      int64_t ijUnderflowIdx = (i * iterations2 + (j - 1));

      matrix[ijIdx] += 7;
      matrix[ijOverflowIdx] += 13;
      matrix[ijUnderflowIdx] += 11;
    }

    for (int64_t j = 4; j < iterations2 - 2; ++j) {
      int iBase = (i - 1) * iterations2;

      // Different constants never overlap
      int i2 = iBase + 2;
      int i4 = iBase + 4;
      matrix[i2]++;
      matrix[i4]--;

      // Within max offsets allowed:
      int ij_0 = iBase + (j + 1);
      int ij_1 = iBase + (j - 1);
      // Max offsets allowed:
      int ij_2 = iBase + (j + 2);
      int ij_3 = iBase + (j - 4);
      // Past max offsets, not allowed:
      int ij_4 = iBase + (j + 3);
      int ij_5 = iBase + (j - 5);

      matrix[ij_0]++;
      matrix[ij_1]--;
      // matrix[ij_2]++;
      // matrix[ij_3]--;
      // matrix[ij_4]++;
      // matrix[ij_5]--;

    }
  }

  printf("%d, %d\n", matrix[0], matrix[iterations * (iterations2 - 1)]);

  return 0;
}
