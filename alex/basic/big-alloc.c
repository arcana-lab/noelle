#include <stdio.h>
#include <string.h>

struct Book {
   char  subject[1024];
   int   book_id;
};

int main (){
  int iter_num = 1000000;

  struct Book *array[iter_num];

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial A");
    b->book_id = 6495407;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial B");
    b->book_id = 6495408;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial C");
    b->book_id = 6495409;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial D");
    b->book_id = 6495410;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial E");
    b->book_id = 6495411;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial F");
    b->book_id = 6495412;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial G");
    b->book_id = 6495413;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial H");
    b->book_id = 6495414;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial I");
    b->book_id = 6495415;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial J");
    b->book_id = 6495416;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial K");
    b->book_id = 6495417;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial L");
    b->book_id = 6495418;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial M");
    b->book_id = 6495419;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial N");
    b->book_id = 6495420;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial O");
    b->book_id = 6495421;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial P");
    b->book_id = 6495422;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial Q");
    b->book_id = 6495423;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial R");
    b->book_id = 6495424;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  for (int i=0; i < iter_num; i++) {
    struct Book *b;
    b = (struct Book*) malloc(sizeof(struct Book));

    strcpy(b->title, "C Programming");
    strcpy(b->author, "Nuha Ali"); 
    strcpy(b->subject, "C Programming Tutorial S");
    b->book_id = 6495425;

    array[i] = b;
  }

  for (int i = iter_num - 1; i >= 0; i--) {
    free(array[i]);
  }

  return 0;
}