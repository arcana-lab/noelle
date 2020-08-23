#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct _NODE {
  int i1;
  int i2;
  int i3;
};
typedef struct _NODE NODE;

int main (int argc, char *argv[]){

  auto NUM_NODES = 10 * argc;
  NODE nodes[NUM_NODES];
  auto nodePtr = nodes;

  auto nodePtrInt = (int64_t)nodePtr;
  for (auto i = 0; i < NUM_NODES - 1; ++i) {
    nodePtrInt += 12;
    auto nodePtrPtr = (NODE *)nodePtrInt;
    nodePtrPtr->i1 = i;
    nodePtrPtr->i2 = i + argc;
    nodePtrPtr->i3 = 1;
  }

  nodePtrInt = (int64_t)nodePtr;
  for (auto i = 0; i < NUM_NODES - 1; ++i) {
    nodePtrInt += 12;
    auto nodePtrPtr = (NODE *)nodePtrInt;
    printf("%d, %d, %d\n", nodePtrPtr->i1, nodePtrPtr->i2, nodePtrPtr->i3);
  }

  return 0;
}

