#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);

  long long int i = 0, s = 1;
  do {
    s += argc * 2;
    if (i == iterations) break;
    i++;
    s += 5;
  } while (i < iterations);

  printf("%lld\n", s);

  return 0;
}
