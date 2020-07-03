#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  float *coord;
  unsigned long long int a;
  unsigned long long int c;
  unsigned long long int w;
} Point;

typedef struct {
  unsigned long long int size;
  unsigned long long int dim;
  Point *p;
} Points;

void computeSum (Points *points){
  points->p[0].c = 3;
}
int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s POINTS DIMS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto dims = atoll(argv[2]);

  /*
   * Allocate the memory.
   */
  Points points;
  points.size = iterations;
  points.dim = dims;
  points.p = (Point *) malloc(sizeof(Point) * points.size);

  for (auto i=0; i < points.size; i++){
    auto point = &(points.p[i]);
    point->coord = (float *)malloc(sizeof(float) * points.dim);
    for (auto j=0; j < points.dim; j++){
      point->coord[j] = j;
    }
    point->w = i;
  }

  /*
   * Hot loop
   */
  computeSum(&points);

  /*
   * Print the results
   */
  printf("%f\n", points.p[0].coord[0]);

  return 0;
}
