#include <stdio.h>

int main (){
  int v1, v2, v1_temp;
  v1 = 0;
  v2 = 0;

  for (int i = 0; i < 100; ++i) {
    v1_temp = v1;
    v1 = v1 + 1;
    v1 = v1 - 3;
    v2 = v2 - v1_temp;
    v2 = v2 * 3;
  }

  printf("%d, %d\n", v1, v2);
  return 0;
}
