//#include <omp.h>
#include <stdio.h>

void block_counter () {
  int COUNT = 0;
  //#pragma omp parallel
  #pragma note noelle independent = 1
  for(int i = 0; i < 16; i++) 
  {
    int x;
    //#pragma omp critical
    #pragma note noelle critical = 1
    {
      /* NOTE(Simone): if this block is not executed one-at-a-time by
       * concurrent threads, stale reads are likely. Test this by
       * commenting out the OpenMP Pragma.
       */
      x = COUNT;
      COUNT++;
      COUNT++;
      COUNT++;
    }
    /* NOTE(Simone): This block is self-contained (has no dependencies
     * outside of the block or which escape), allowing it to be executed
     * safely in parallel by many threads.
     */
    int y = x + 1;
    int z = y * x;
    printf("%d, %d, %d\n", x, y, z);
  }
}
/*
void loop_counter () {
  int NUMS[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  //#pragma omp parallel for
  #pragma note noelle independent = 1 ordered = 1
  for (int i = 0; i < 100; i++) {
    int count_modulo_10;
    //#pragma omp critical
    #pragma note noelle independent = 0 ordered = 0
    {
      NUMS[(i % 10)]++;
      count_modulo_10 = NUMS[(i % 10)];
    }
    int half = count_modulo_10 / 2;
    if (half > 5) {
      printf("on %d hit %d for %d", i, count_modulo_10, (i % 10));
      #pragma note noelle independent = 0
      {
        NUMS[(i % 10)] = 0;
      }
    }
  }
}
*/
int main () {
  block_counter();
//  loop_counter();
  return 0;
}
