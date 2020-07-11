#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;

  long long int *array = (long long int *) calloc(argc * 10, sizeof(long long int));

  // Loop id 0, nice, simple, just an IV governed loop
  for (auto i = 0; i < iterations; ++i){
    array[0] -= argc;

    // To prevent analyses from promoting array to registers
    array[argc * 3] = array[0];

    // To prevent the while true loops from never exiting and to avoid relying on easy-to-analyze IVs)
    int maxSubIters = argc;

    // Loop id 1, not so nice. Exits to loop id 2 OR loop id 3
    while (true) {
      array[1] -= argc * i;

      // One of go-to pair producing Loop id 3 NOT identified by LLVM
      LOOP3_PART1:

        // Breaks to loop id 2
        if (maxSubIters-- < 0) break;

        array[2] -= argc;

        if (array[2] < 0) goto LOOP3_PART2;
    }

    // Loop id 2. Either loop id 1 or loop id 3 goes to here
    while (true) {
      array[1] += argc * i;

      // Other of go-to pair producing Loop id 3 NOT identified by LLVM
      LOOP3_PART2:

        // Breaks to latch of loop id 0
        if (maxSubIters-- < 0) break;

        array[2] += argc;

        // To produce an SCC in loop 3's structure
        printf("Iteration from loop 3. Within loop 0 iteration %d\n", i);

        if (array[2] > 0) goto LOOP3_PART1;
    }
  }

  printf("%lld, %lld, %lld, %lld\n", array[0], array[1], array[2], array[argc * 3]);

  return 0;
}
