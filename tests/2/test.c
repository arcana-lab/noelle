#include <stdio.h>

int f (int *arr){
  arr[0] += 2;
  return arr[0]; 
}

int main (){
  int arr[] = {1,2};
  int q = f(arr);
  printf("%d\n", q);

  return 0;
}

