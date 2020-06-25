#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int myF (int n){
  if (n >= 1){
    exit(0);
  }

  return myF(n + 1);
}

int main (int argc, char *argv[]){
  int v2;
  v2 = ((argc * 5) / 2) + 1;

  int t=0;
  int i;
  while (1){
    t += myF(i);
    i++;
  }

  printf("%d, %d\n", t, v2);
  return 0;
}
