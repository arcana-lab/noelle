#include <stdio.h>

int main (){
  int x;

  for (int i = 0; i < 1000; ++i) {
    int iter = i % 30;
    int a = i * iter + iter - 5 + i;
    int b = a * iter + i;
    int c = b * a + 10;
    int d = c * b * a + iter;
    int e = a * d + i;
    int f = d * c * a + iter * e;
    int g = f * e;
    x = g * iter;
    printf("Hello, world! %d", i);
    printf("Hello Simone!");
  }
  printf("%d\n", x);

  return 0;
}

