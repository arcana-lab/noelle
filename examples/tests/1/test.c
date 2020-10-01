#include <stdio.h>

void myG (void){

}

void myF (void){
  myG();
}

int main (){
  printf("This is fun\n");
  myF();
  return 0;
}
