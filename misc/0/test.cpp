#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

class MyClass {
  public:
    int64_t a;
};

MyClass * myf (void){
  auto d = new MyClass();
  d->a = 10;

  return d;
}

int main (){
  auto p = myf();
  printf("%ld\n", p->a);

  return 0;
}
