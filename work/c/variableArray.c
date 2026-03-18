#include "variableArray.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int size;
    printf("数组初始大小：");
    scanf("%d",&size);
    Array a = array_creat(size);
    int number,cnt = 0;
    printf("你的数组初始大小为：%d\n",array_size(&a));
    printf("第%d个数填(输入-1停止)：",cnt+1);
    scanf("%d",&number);
    while(number != -1) {
        if (cnt+1 >= a.size) {
            array_inflate(&a,BLOCK_SIZE);
            break;
        }
        *array_at(&a,cnt) = number;
        cnt++;
        printf("第%d个数填(输入-1停止)：",cnt+1);
        scanf("%d",&number);
    }
    printf("现在你的数组大小是：%d\n",array_size(&a));
    int index;
    printf("输入你想查询的数字位置：");
    scanf("%d",&index);
    while (index != -1) {
        if(index >= array_size(&a)) {
            printf("禁止偷看！\n");
        } else {
            printf("第%d位的数字是%d\n",index,*array_at(&a,index-1));
        }
        printf("输入你想查询的数字位置：");
        scanf("%d",&index);
    }

    return 0;
}
Array array_creat(int size){
    Array a;
    size = (size/BLOCK_SIZE+1)*BLOCK_SIZE;
    a.array = (int*)malloc(sizeof(int)*size);
    a.size = size;
    return a;
}

void array_free(Array *a){
    free(a->array);
    a->array = NULL;
    a->size = 0;
}

int array_size(Array *a){
    return a->size;
}

int* array_at(Array *a,int index){
    return &(a->array[index]);
}

void array_inflate(Array *a,int more_size){
    int*p = (int*)malloc(sizeof(int)*(a->size+more_size));
    for (int i = 0;i <= a->size;i++) {
        p[i] = a->array[i];
    }
    free(a->array);
    a->array = p;
    a->size += more_size;
}