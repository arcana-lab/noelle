#include <stdio.h>
#include <stdlib.h>

struct S {
   int x;
   int y;
};

int test(S *opaque, int tripcount) {
  int result1 = 0;
  int result2 = 0;
  for (int i = 0; i < tripcount; i++) {
    int x1 = opaque->x;
    result1 += x1;
    opaque->y = result1;

    int x2 = opaque->x;
    result2 = x2;
    opaque->y = result2;
  }
  return result1 + result2 + opaque->y;
}


int main (int argc, char *argv[]){
  S *opaque = (S *) malloc(sizeof(S));
  opaque->x = argc;
  opaque->y = argc * 2;
  int result = test(opaque, argc + 20);
  printf("%d\n", result);
  return 0;
}

