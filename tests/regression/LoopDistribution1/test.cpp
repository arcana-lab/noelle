#include <stdio.h>

int main (){
  int x;

  for (int i = 0; i < 1000; ++i) {
    if (i != 0 && i % 70 == 0) {
      printf("Failed"); 
      return 0;
    }
    x = i % 30;
    printf("Hello, world!");
  }
  printf("%d\n", x);

  return 0;
}

