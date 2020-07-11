#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int sccCausing1 = 0;
int sccCausing2 = 0; 
int doRecurse2 = 0;
void func2_recurse();
void func2() {
  if (sccCausing2++ % 10 == 0) {
    sccCausing2 = 1;
    func2_recurse();
  }
}
void func2_recurse() {
  if (doRecurse2) func2();
}

int main (int argc, char *argv[]){

  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;

  // The goal here is that func2 depends on sccCausing1 (because of loop control dependencies)
  // though func2 appears first in the control flow graph
  do {
    func2();
  } while (sccCausing1++ < iterations);

  printf("%d, %d\n", sccCausing1, sccCausing2);

  return 0;
}
