#include <stdio.h>

int main (){
  double v1, v2;
  int v3, v4;
  int printVar;
  v1 = 111.0;
  v2 = 3;
  v3 = 7;
  v4 = 11;

  printVar = 0;

  for (int i = 0; i < 100; ++i) {
    v1 -= 1;
    v2 += v1 - 15;
    v2 += 13;
    v2 /= 2;
    v3 = v2 + 17;
    v4 = v3 - 23;

    printVar += printf("Iteration\n");
  }

  printf("Number of bytes printed out in total:%d\n", printVar);
  printf("%1f, %1f, %d, %d\n", v1, v2, v3, v4);
  return 0;
}
