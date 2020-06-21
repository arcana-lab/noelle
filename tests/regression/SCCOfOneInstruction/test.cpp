#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// toggleGlobal should become a SCC of a single call instructions
int global = 0;
bool preventInline = true;
int toggleGlobal() {
  if (!preventInline) toggleGlobal();
  return global = global == 1 ? 0 : 1;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);

  int s = 0;
  for (int i = 0; i < iterations; ++i) {
    s += iterations;
    s -= toggleGlobal();
  }

  printf("%d\n", s);

  return 0;
}
