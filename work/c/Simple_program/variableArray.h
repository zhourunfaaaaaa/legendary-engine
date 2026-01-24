#ifndef _VARIABLE_ARRAY_H_
#define _VARIABLE_ARRAY_H_

// 定义结构体
typedef struct {
    int *array; // 指向堆内存的指针
    int size;   // 当前数组的大小（元素个数）
} Array;

// 函数声明

// 创建数组
Array array_creat(int init_size);

// 释放数组
void array_free(Array *a);

// 获取大小
int array_size(const Array *a); // 加上 const，表示我不修改它

// 访问元素（核心函数）
int* array_at(Array *a, int index);

// 自动扩容（内部接口，也可以暴露出来）
// 返回值：1 表示成功，0 表示失败
int array_inflate(Array *a, int more_size);

#endif