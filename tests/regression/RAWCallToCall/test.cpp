#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cassert>

typedef struct Point{
  int x;
  int y;
  int arr[10];
} Point;

typedef struct PointArr{
    int p;
    int q;
  Point **pointArr;
} PointArr;

typedef struct IndirecRefToPoint {
    Point *p;
} IndirecRefToPoint;

int __attribute__((noinline)) SqrtOfSum(IndirecRefToPoint *indirectRef){
  Point *p = indirectRef->p;
  return sqrt(p->x + p->y);
}



int main (int argc, char *argv[]){
  if (argc < 1){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  IndirecRefToPoint indirectRef;
  PointArr *ptrArr;
  ptrArr = (PointArr*)malloc(sizeof(PointArr));
  Point *p = (Point *)malloc(sizeof(Point));
  indirectRef.p = p;
  auto iterations = atoi(argv[1]);
  auto x = atoi(argv[2]);
  auto y = atoi(argv[3]);
  ptrArr->pointArr = (Point**)malloc(sizeof(void *)*iterations);
  for (auto i=0;i < iterations; i++){
    Point *tmp = (Point *)malloc(sizeof(Point));
    tmp->x = x++;
    tmp->y = y++;
    ptrArr->pointArr[i] = tmp;
  }
  if (iterations < 0) {
    fprintf(stderr, "USAGE: %s requires a position iteration count\n", argv[0]);
    return -1;
  }

  int result = 0;
  for (auto i=0;i < iterations; i++){
    memcpy(p, ptrArr->pointArr[i], sizeof(Point));
    result += SqrtOfSum(&indirectRef);
  }

  int actualResult = 0;
  for (auto i=0;i < iterations; i++){
    p->x = ptrArr->pointArr[i]->x;
    p->y = ptrArr->pointArr[i]->y;
    actualResult += SqrtOfSum(&indirectRef);
  }

  assert(result == actualResult && "results do not match");
  printf("%d\n", result);
  return 0;
}
