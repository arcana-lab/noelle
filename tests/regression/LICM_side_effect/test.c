#include <stdio.h>
#include <stdlib.h>


void myF(int num, int* ctr, char * class, double *xcr) {
  double xcrref[5], xcrdif[5];
  *ctr = 1;

  if (num == 12) {
    *class = 'S';
    xcrref[0] = 1.7034283709541311e-01;
    xcrref[1] = 1.2975252070034097e-02;
    xcrref[2] = 3.2527926989486055e-02;
    xcrref[3] = 2.6436421275166801e-02;
    xcrref[4] = 1.9211784131744430e-01;
  } else if (num == 16) {
    *class = 'B';
      xcrref[0] = 1.4233597229287254e+03;
      xcrref[1] = 9.9330522590150238e+01;
      xcrref[2] = 3.5646025644535285e+02;
      xcrref[3] = 3.2485447959084092e+02;
      xcrref[4] = 3.2707541254659363e+03;
  } else {
    *ctr = 0;
  }

  for (int m = 0; m < 5; m++) {
    xcrdif[m] = ((xcr[m]-xcrref[m])/xcrref[m]);
  }

  double epsilon = 1.0e-08;

  if (*class != 'U') {
    printf(" Comparison of RMS-norms of solution error\n");
  } else {
    printf(" RMS-norms of solution error\n");
  }

  for (int i = 0; i < 5; i++) {
    if (*class == 'U') {
      printf("class == U\n");
    } else if (xcrdif[i] > epsilon) {
      /* BUGGED, this store is being hoisted */
      *ctr = 0;
      printf("Over epsilon!\n");
    } else {
      printf("It's good\n");
    }
  }
}

int main(int argc, char *argv[]) {

  int* ctr = (int*) malloc(sizeof(int));

  double arr[5];

  *ctr = 1;
  int num = atoll(argv[1]);
  for (int i = 0; i < 5; i++) {
    arr[i] = atof(argv[i+2]);
  }
  char * class = (char*) malloc(sizeof(char));

  myF(num, ctr, class, arr);

  if (*ctr == 0) {
    printf("but not good?\n");
  } else if (*ctr == 1) {
    printf("still good\n");
  } else {
    printf("Also, quite strange\n");
  }


  free(ctr);
  return 0;
}

