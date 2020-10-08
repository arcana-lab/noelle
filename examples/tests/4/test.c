#include <stdio.h>

void myG1 (void){
  printf("Inside myG1\n");
}

void myG2 (void){
  printf("Inside myG2\n");
}

void myG3 (void){
  printf("Inside myG3\n");
}

void myF (void (*funcToInvoke) (void)){
  (*funcToInvoke)();
  myG1();
}

int main (int argc, char *argv[]){
  printf("Start\n");

  if (argc > 2){
    myF(myG1);
  } {
    myF(myG2);
  }
  printf("End\n");
  return 0;
}
