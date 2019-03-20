#include <stdio.h>
#include <stdint.h>

extern int a;

int main (int argc, char *argv[]){

  a = 0;

  for (int i = 0; i < 100; i++) {
    a++;
  }
  
  printf("%d\n", a);
  return 0;
}
