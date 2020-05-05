#include <stdio.h>
#include <stdint.h>

int main (int argc, char *argv[]){
  int v1 = 7;

  int i, j = 4;
  int arr[] = {1,6,2,8,-4};
  for (i = 0; i < argc * 10000; ++i) {
    j += i;

    v1 += j - arr[i % 5];
  }

  printf("%d, %d, %d\n", i, j, v1);
  return 0;
}
