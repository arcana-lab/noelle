#include <stdio.h>
#include <stdint.h>

int main (int argc, char *argv[]){
  int j = argc;
  int k = 0;
  int nonIV = ((argc * 5) / 2) + 1;

  for (uint32_t i = 0; i < 100; ++i) {

    j = j + 1;
    j = j - 3;

    k += (argc * (argc + 1));

    nonIV = nonIV * 3 + j;
    nonIV = nonIV / 2;
  }

  printf("%d, %d, %d\n", j, k, nonIV);
  return 0;
}
