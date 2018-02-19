#include <stdio.h>

int main (){
  int v1, v2;
  v1 = 0;
  v2 = 0;

  for (int i = 0; i < 10000; ++i) {
    v1 = v2 + 1;
    v2 = v1 + 5;
  }

  return 0;
}

