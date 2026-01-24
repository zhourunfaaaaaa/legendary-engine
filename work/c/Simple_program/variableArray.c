#include "variableArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // 必须引用，用于 memcpy

const int BLOCK_SIZE = 20;

Array array_creat(int init_size)
{
    Array a;
    a.size = init_size;
    a.array = (int*)malloc(sizeof(int) * init_size);
    
    // 🛡️ Init failed check
    if (a.array == NULL) a.size = 0;
    
    return a;
}

void array_free(Array *a)
{
    if (a == NULL) return;
    if (a->array != NULL) free(a->array);
    
    // 🛡️ Prevent dangling pointer
    a->array = NULL;
    a->size = 0;
}

// --- 扩容策略核心区域 ---

/**
 * 策略 1: 标准库 realloc (推荐)
 * 优点: 可能原地扩容，效率高，代码少
 */
int array_inflate(Array *a, int more_size)
{
    if (more_size <= 0) return 1;
    int new_size = a->size + more_size;

    // 🛡️ Use temp ptr to protect old data
    int *p = (int*)realloc(a->array, sizeof(int) * new_size);
    
    if (p == NULL) return 0; // Fail safe
    
    a->array = p;
    a->size = new_size;
    return 1;
}

/**
 * 策略 2: 手动 malloc + memcpy (你要求的另一种方法)
 * 原理: 开新房 -> 搬家(Copy) -> 拆旧房(Free)
 */
/*
int array_inflate(Array *a, int more_size)
{
    if (more_size <= 0) return 1;
    int new_size = a->size + more_size;

    // 1. Alloc new block
    int *p = (int*)malloc(sizeof(int) * new_size);
    if (p == NULL) return 0; // Fail safe

    // 2. Copy data (Efficient than loop)
    // 🛡️ Copy bytes = count * sizeof(type)
    memcpy(p, a->array, sizeof(int) * a->size);

    // 3. Free old block
    free(a->array);

    // 4. Update
    a->array = p;
    a->size = new_size;
    return 1;
}
*/

// --- 结束 ---

int* array_at(Array *a, int index)
{
    if (index >= a->size) {
        int needed = (index / BLOCK_SIZE + 1) * BLOCK_SIZE - a->size;
        // 🛡️ Check inflation result
        if (array_inflate(a, needed) == 0) return NULL;
    }
    return &(a->array[index]);
}

int array_size(const Array *a)
{
    return a ? a->size : 0;
}

int main()
{
    Array a = array_creat(100);
    // 🛡️ Init check
    if (a.array == NULL) return 1;

    printf("Initial: %d\n", array_size(&a));
    
    int number = 1, cnt = 0;
    while (number != -1) {
        scanf("%d", &number);
        if (number != -1) {
            // 🛡️ Safe Dereference: Check NULL before write
            int *ptr = array_at(&a, cnt++);
            if (ptr != NULL) {
                *ptr = number;
            } else {
                printf("Err: OOM at %d\n", cnt);
                break;
            }
        }
    }

    // Safe read check
    int *first = array_at(&a, 0);
    if (first) printf("First: %d\n", *first);

    array_free(&a);
    return 0;
}