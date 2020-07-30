#include <stdio.h>

void myG (void){

}

void myF (int v){
  if (v == 0){
    return ;
  }
  myF(v-1);
  myG();
  myG();
}

int main (){
  printf("Start\n");
  myF(5);
  printf("End\n");
  return 0;
}
