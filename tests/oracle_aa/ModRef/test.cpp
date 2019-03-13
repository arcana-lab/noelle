#include <stdio.h>
#include <stdint.h>

int a[100];

void g(int i);
void f(int i);

void g(int i) {
  f(i);

}

void f(int i) {
  a[i] = a[i+1] + a[i+1]; 
}

int main (int argc, char *argv[]){

  for (int i = 0; i < 100; i++) {
    a[i] = i + 4;
  }

  for (int i = 0; i < 98; i++) {
    g(i);
    a[i+2] += a[i];
  }

  printf("%d, %d\n", a[99], a[50]);
  return 0;
}
