#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void myF (int n){
  if (n >= 1){
    exit(0);
  }
}

void myG (int n){
  if (n <= 10){
    exit(0);
  }
}

int main (int argc, char *argv[]){
  int v1, v2;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;

  void (*ptrToF)(int n);
  if (argc == 0){
    ptrToF = myF;
  } else {
    ptrToF = myG;
  }

  for (uint32_t i = 0; i < 100; ++i) {

    // SCC 1
    v1 = v1 + 1;
    v1 = v1 - 3;

    ptrToF(argc);

    // SCC 2
    v2 = v2 * 3 + v1;
    v2 = v2 / 2;
  }

  printf("%d, %d\n", v1, v2);
  return 0;
}
