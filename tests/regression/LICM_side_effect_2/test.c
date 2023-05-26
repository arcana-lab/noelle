#include <stdio.h>
#include <stdlib.h>

void myG(int *ctr) {
  *ctr = 0;
}

void myG2(int *ctr) {
  return;
}

void myF(int num,
         int *ctr,
         char *class,
         double *xcr,
         void (*myGPtr)(int *ctr)) {
  *ctr = 1;
  for (int i = 0; i < 5; i++) {
    if (*class == 'U') {
      printf("class == U\n");
    } else if (xcr[i] <= 0.00000001F) {
      /* BUGGED, this store is being hoisted */
      (*myGPtr)(ctr);
      printf("Over epsilon!\n");
    } else {
      printf("It's good\n");
    }
  }
}

int main(int argc, char *argv[]) {

  int *ctr = (int *)calloc(1, sizeof(int));

  double arr[5];

  *ctr = 1;
  int num = atoll(argv[1]);
  for (int i = 0; i < 5; i++) {
    arr[i] = atof(argv[i + 2]);
  }
  char *class = (char *)calloc(1, sizeof(char));

  if (argc > 1000) {
    myF(num, ctr, class, arr, myG2);
  } else {
    myF(num, ctr, class, arr, myG);
  }

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
