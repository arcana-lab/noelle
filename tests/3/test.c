#include <stdio.h>

int f (int *arr){
  for (int i = 0; i < 2; ++i) {
  	arr[i] += arr[(i + 1) % 2];
  }

  return arr[0]; 
}

int main (){
  int arr[] = {1,2};
  int q = f(arr);
  printf("%d\n", q);

  return 0;
}

