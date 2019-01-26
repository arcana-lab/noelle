#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  int randomVariableInTheWay;
  float *array;
} Point;

typedef struct {
  int iters;
  int dim;
  Point *p;
} Points;

float readOnly (Point &p, Point &q, int dim) {
  float result=0.0;
  for (auto i = 0; i < dim; i++)
    result += (p.array[i] - q.array[i])*(p.array[i] - q.array[i]);
  return result;
}

void doPrint (Points *p) {
  printf("%d\n", p->dim);
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations < 1) return -1;

  Points points;
  points.dim = 1000;
  points.iters = iterations;

  Points *pointsTest = new Points();
  pointsTest->dim = iterations;
  doPrint(pointsTest);

  float* block = (float*)malloc(iterations*points.dim*sizeof(float));
  points.p = (Point *)malloc(iterations*sizeof(Point));

  for (int i = 0; i < iterations * points.dim; ++i) {
    block[i] = i * 1.6;
  }
  for (int i = 0; i < iterations; ++i) {
    points.p[i].array = &block[i*points.dim];
  }

  auto is_valid = (bool *)calloc(iterations, sizeof(bool));
  auto values = (int *)malloc(iterations * sizeof(int));

  Point startPoint = points.p[0];
  float accumulation = 0;
  for (auto i=0; i < iterations; ++i) {
    float someValue = readOnly(points.p[i], startPoint, points.dim);
    float otherValue = startPoint.array[0];
    if (someValue < otherValue) {
      is_valid[i] = 1;
      accumulation += someValue - otherValue;
    } else {
      values[i] += otherValue - someValue;
    }
  }

  printf("%.2f, %d\n", accumulation, is_valid[iterations/2], values[iterations/2]);

  return 0;
}
