#include <stdio.h>
#include <stdint.h>

int a[100];

int main (int argc, char *argv[]){

  for (int i = 0; i < 100; i++) {
    a[i] = i + 4;
  }

  for (int i = 0; i < 98; i++) {
    a[i] += a[i-1];
  }

  printf("%d, %d\n", a[99], a[50]);
  return 0;
}
