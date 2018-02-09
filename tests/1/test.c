#include <stdio.h>

int f (int p1){
  int arr[] = {1,2};
  int *b = arr;
  int e;

  b++;
  e = *b;

  printf("%d\n", e);
 
  return 0; 
}

int main (){
  int n=0;
  int q = f(n);
  printf("%d\n", q);

  return 0;
}

