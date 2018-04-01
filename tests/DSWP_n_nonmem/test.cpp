#include <stdio.h>

int main (){
  int v1, v2, v3, v4;
  v1 = 111;
  v2 = 3;
  v3 = 7;
  v4 = 11;

  for (int i = 0; i < 100; ++i) {
    v1 -= 1;
    v2 += v1 - 15;
    v2 += 13;
    v2 /= 2;
    v3 = v2 + 17;
    v4 = v3 - 23;
  }

  printf("%d, %d, %d, %d\n", v1, v2, v3, v4);
  return 0;
}
