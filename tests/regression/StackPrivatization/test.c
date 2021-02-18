#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]){

  if (argc <= 1){
    fprintf(stderr, "USAGE: %s ITERATIONS\n", argv[0]);
    return 1;
  }
  long long int iterations = atoll(argv[1]);

  long long int t = 0;
  for (long long int i=0; i < iterations; i++){
    long long int a[100];
    memset(a, 0, sizeof(long long int) * 100);
    int index = i % 100;
    a[index] = iterations + i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;
    a[index] = (long long int)sqrt((double)a[index]);
    a[index] += 10;
    a[index] *= i;

    int index2 = i % 100;
    a[index2] = iterations * i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;
    a[index2] = (long long int)sqrt((double)a[index2]);
    a[index2] += 10;
    a[index2] *= i;

    for (int j=0; j < 100; j++){
      t += a[j];
    }
  }

  printf("%d\n", t);

  return 0;
}
