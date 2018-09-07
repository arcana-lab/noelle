#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  float *coord;
  unsigned long long int a;
  unsigned long long int c;
  unsigned long long int w;
} Point;

float computeDist (Point p1, Point p2, int dim){
  float result = 0.0;
  for (auto i=0; i < dim; i++){
    auto toAdd = (p1.coord[i] - p2.coord[i]) * (p1.coord[i] - p2.coord[i]);
    result += toAdd;
  }

  return result;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s DIMS\n", argv[0]);
    return -1;
  }
  auto dims = atoll(argv[1]);

  /*
   * Allocate the memory.
   */
  Point p1;
  Point p2;
  p1.coord = (float *)malloc(sizeof(float) * dims);
  p2.coord = (float *)malloc(sizeof(float) * dims);
  for (auto j=0; j < dims; j++){
    p1.coord[j] = j + dims + 1;
    p2.coord[j] = j + 1;
  }

  /*
   * Hot loop
   */
  auto r = computeDist(p1, p2, dims);

  /*
   * Print the results
   */
  printf("%f\n", r);

  return 0;
}
