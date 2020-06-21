#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int dont = 0;
void iPreventTheBlockFromCollapsingAlso () ;
void iPreventTheBlockFromCollapsing() {
  if (dont) iPreventTheBlockFromCollapsingAlso();
}
void iPreventTheBlockFromCollapsingAlso() {
  if (dont) iPreventTheBlockFromCollapsing();
}

int main (int argc, char *argv[]) {

  if (argc < 2){
    fprintf(stderr, "USAGE: %s ITERATIONS\n", argv[0]);
    return -1;
  }

  auto iterations = atoi(argv[1]);

  int x = 3;
  for (int i = 0; i < iterations; ++i) {
    int xPlus;
    if (x < argc * 3) {
      xPlus = 1;
      iPreventTheBlockFromCollapsing();
    } else {
      xPlus = 2;
      iPreventTheBlockFromCollapsingAlso();
    }

    x += xPlus;
  }

  printf("value = %d\n", x);

  return 0;
}
