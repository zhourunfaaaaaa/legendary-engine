#ifndef _ARRAY_
#define _ARRAY_

typedef struct {
    int *array;
    int size;
} Array;

const int BLOCK_SIZE = 20;

Array array_creat(int size);
void array_free(Array *a);
int array_size(Array *a);
int* array_at(Array *a,int index);
void array_inflate(Array *a,int more_size);

#endif