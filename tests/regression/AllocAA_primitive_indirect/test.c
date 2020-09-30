#include <stdio.h>
#include <stdlib.h>

int * i;

int* myFunction(int num) {
   return (int *) malloc( sizeof(int) * (num * num));
}

int* myFunction2(int num) {
  return (int *) malloc( sizeof(int) * (num * 3));
}

int process(int num) {
  i = (*(num % 2 == 1 ? myFunction : myFunction2))(num);
  return num;
}


int main(int argc, char *argv[]) {
  int num = atoi(argv[1]);
  process(num);
  printf("%d,%d", num, *i);
  return 0;
}
