#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct Vector {
  inline Vector(void) : x(-99999), y(-99999), z(-99999) { ; }
  inline Vector(double newv)  // allow Vector v = 0; etc.
    : x(newv), y(newv), z(newv) { ; }
  inline Vector(double x, double y, double z) : x(x), y(y), z(z) { ; }

  inline double operator[](int i) {
    return i==0 ? x : i==1 ? y : i==2 ? z : -1;
  }

  inline void operator+=(const Vector &v2) {
    x += v2.x;
    y += v2.y;
    z += v2.z;
  }

  inline void operator-=(const Vector &v2) {
    x -= v2.x;
    y -= v2.y;
    z -= v2.z;
  }

  inline friend Vector operator+(const Vector& v1, const Vector& v2) {
    return Vector( v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
  }

  inline friend Vector operator-(const Vector& v1, const Vector& v2) {
    return Vector( v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
  }

  inline double length2(void) const {
    return (x*x + y*y + z*z);
  }

  double x, y, z;
};

int main (int argc, char *argv[]){

  if (argc < 2) {
    fprintf(stderr, "USAGE: %s POINTS\n", argv[0]);
    return 1;
  }

  int numAtoms = atoi(argv[1]);
  if (numAtoms < 1) {
    printf("Too few atoms. Requires at least 1\n");
  }

  double accumulation = 0;
  for (int i = 0; i < numAtoms * numAtoms; ++i) {
    Vector both = Vector(i);

    for (int j = 0; j < numAtoms * numAtoms; ++j) {
      int k = i / numAtoms + j;
      if (k % 71 == 0) {
        both += Vector(1, 1, 1);
      } else if (k % 131 == 0) {
        both += Vector(-1, -1, -1);
      }
    }

    accumulation += both.length2() / Vector(10 * (i + 1)).length2();
  }
  
  printf("%d, %.4f\n", numAtoms, accumulation);
}
