#include <stdio.h>

void myG (void){

}

void myF (void){
  myG();
  myG();
}

int main (){
  printf("Start\n");
  myF();
  printf("End\n");
  return 0;
}
