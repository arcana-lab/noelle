#include <stdio.h>

int f(int v) {
  return v + 2;
}

int g(int printVar, int v1, int v2, int (*f_f)(int)) {
  for (int i = 0; i < 100; ++i) {
    v1 -= 1;
    printVar += (*f_f)(v1);
    v1 += printVar;
    printVar -= v1;
    v2 += v1 + printVar;
  }

  printf("Final output: %d %d %d\n", printVar, v1, v2);
  return 0;
}

int main (){
  int v1, v2;
  int printVar;
  v1 = 111;
  printVar = 0;
  v2 = 4;
  auto *f_f = &f;

  g(printVar, v1, v2, f_f);

  return 0;
}
