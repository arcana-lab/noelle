#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct _NODE {
  int i1;
  int i2;
};
typedef struct _NODE NODE;

int main (int argc, char *argv[]){

  auto NUM_NODES = 10 * argc;
  NODE nodes[NUM_NODES];
  auto nodePtr = nodes;

  auto nodePtrInt = (int64_t)nodePtr;
  for (auto i = 0; i < NUM_NODES; ++i) {
    nodePtrInt += 8;
    auto nodePtrPtr = (NODE *)nodePtrInt;
    nodePtrPtr->i1 = i;
    nodePtrPtr->i2 = i + argc;
  }

  nodePtrInt = (int64_t)nodePtr;
  for (auto i = 0; i < NUM_NODES; ++i) {
    nodePtrInt += 8;
    auto nodePtrPtr = (NODE *)nodePtrInt;
    printf("%d, %d\n", nodePtrPtr->i1, nodePtrPtr->i2);
  }

  return 0;
}

