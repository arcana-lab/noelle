#include <stdio.h>
#include <stdlib.h>

char *weirdPtr;

void myG2(void);
void myF(void (*funcToInvoke)(void));

void myG1(void) {
  printf("Inside myG1\n");
  if (((int)weirdPtr) == rand()) {
    myG2();
  }
}

void myG2(void) {
  printf("Inside myG2\n");
  if (((int)weirdPtr) == rand()) {
    myF(myG1);
  }
}

void myG3(void) {
  printf("Inside myG3\n");
}

void myG4(int a) {
  printf("Inside myG4 %d\n", a);
}

void myF(void (*funcToInvoke)(void)) {
  (*funcToInvoke)();
}

int main(int argc, char *argv[]) {
  printf("Start\n");

  weirdPtr = (char *)myG4;
  if (argc > 2) {
    myF(myG1);
  }
  { myF(myG2); }
  printf("End\n");
  return 0;
}
