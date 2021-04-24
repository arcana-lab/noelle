#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Book {
   char  title[50];
   char  author[50];
   char  subject[100];
   int   book_id;
};

int main (){

  int iter_num = 1000000;

  printf("Start Malloc\n");

  struct Book *array[iter_num];

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial");
    b->book_id = 6495407;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  printf("End\n");
  return 0;
}