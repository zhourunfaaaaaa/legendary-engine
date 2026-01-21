// 只可读入正整数的可变数组
#include "variableArray.h"
#include <stdio.h>
#include <stdlib.h>

const BLOCK_SIZE = 20;

Array array_creat(int init_size)
{
    Array a;
    a.size = init_size;
    a.array = (int*)malloc(sizeof(int)*init_size);
    return a;
}

void array_free(Array *a)
{
    free(a->array);
    a->array = NULL;
    a->size = 0;
}

int* array_at(Array *a,int index)
{
    if (index > a->size) {
        array_inflate(a,(index/BLOCK_SIZE+1)*BLOCK_SIZE - a->size);
    }
    return a->array[index];
}

int array_size(Array *a)
{
    return a->size;
}

void array_inflate(Array *a,int more_size)
{
    int *p = (int*)malloc(sizeof(int)*(a->size + more_size));
    for (int i = 0;i < a->array;i++){
        p[i] = a->array[i];
    }
    a->size += more_size;
    a->array = p;
}

int main()
{
    Array a = array_creat(100);
    printf("%d\n",array_size(&a));
    int number = 1;
    int cnt = 0;
    while (number != -1) {
        scanf("%d",&number);
        if (number != -1) {
            *array_at(&a,cnt++) = number;
        }
    }

    printf("%d\n",*array_at(&a,0));
    array_free(&a);

    return 0;
}