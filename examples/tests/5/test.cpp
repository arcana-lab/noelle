#include <stdio.h>

class MyClass {
  public:
    MyClass ()
      : a{10} {};

  int a;
};

MyClass o;

void myF (int v){
  if (v == 0){
    return ;
  }
  myF(v-1);
}

int main (){
  printf("Start\n");
  myF(5);
  printf("End\n");
  return 0;
}
