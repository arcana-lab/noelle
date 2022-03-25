#include <stdio.h>
#include <stdlib.h>

int main (){
  printf("Start\n");

  int *p = (int *)malloc(sizeof(int));
  *p = 0;

  for (int i=0; i < 10; i++){
    *p = 1;
    printf("%d\n", i);
  }

  printf("  Result = %d\n", *p);

  printf("End\n");
  return 0;
}
