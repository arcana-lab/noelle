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

  int x = 3, y = 0;
  for (int i = 0; i < iterations; ++i) {
    int xPlus;
    if (x < argc * 3) {
      xPlus = 1;
      iPreventTheBlockFromCollapsing();
    } else {
      xPlus = 2;
      iPreventTheBlockFromCollapsingAlso();
    }

    for (auto j = 5; j < iterations + 5; ++j) {
      xPlus += j / (j - 1);
      xPlus += sqrt(3 + j);
    }

    int yPlus = 0;
    for (auto j = 10; j < iterations + 10; ++j) {
      yPlus += j / (j - 1);
      yPlus += sqrt(3 + j);
    }

    x += xPlus;
    y += yPlus;
  }

  printf("value = %d\n", x);

  return 0;
}
