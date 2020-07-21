#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void func1();
void func2();

int sccCausing1 = 0;
void func1() {
  if (sccCausing1++ > 2) {
    sccCausing1 = 0;
    func2();
  }
}

int sccCausing2 = 1; 
void func2() {
  if (sccCausing2++ > 2) {
    sccCausing2 = 0;
    func1();
  }
}

int main (int argc, char *argv[]){

  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;

  LOOP0:
  auto maxIters2 = iterations;
  int i = 0, j = 0, k = 0;
  for (i = 0; i < iterations; ++i) {

    // Introduce a control flow path from loop 0 to loop 2
    if (maxIters2 == 0) {
      maxIters2 = iterations;
    } else {
      j = 0;
      goto LOOP2;
    }

    LOOP1: 
    for (j = 0; j < argc; ++j) {
      func1();

      // LLVM doesn't even identify this as a loop...
      LOOP2:
      for (k = 0; k < argc; ++k) {
        func2();

        // Introduce a control flow path from loop 2 to loop 0
        if (maxIters2-- == 0) goto LOOP0_NEXT_ITER;
      }
    }

    LOOP0_NEXT_ITER:
    continue;
  }

  printf("%d, %d\n", sccCausing1, sccCausing2);

  return 0;

}