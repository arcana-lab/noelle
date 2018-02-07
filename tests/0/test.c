#include <stdio.h>

int f (int p1){
  int a, b, c;
  a = 1;
  b = 2;
  c = 3;

  int d = a + b + c + p1;
  int f = d + b + c;
  printf("%d\n", f);
 
  return 0; 
}

int main (){
  int n=0;
  int q = f(n);
  printf("%d\n", q);

  return 0;
}

