#include <stdio.h>

char* mycpy(char* dst, const char* src) {
    // 1. 好的习惯是先检查参数是否为 NULL（防御性编程）
    if (dst == NULL || src == NULL) return dst;

    char* ret = dst; // 记住起始地址用于返回

    // 2. 核心拷贝逻辑
    // 注意：while 后面有一个分号，表示循环体为空
    while ((*dst++ = *src++) != '\0') ; 

    return ret;
}

int main() {
    char s1[10]; // 确保目标空间足够大
    char s2[] = "abc";
    
    mycpy(s1, s2);
    printf("Result: %s\n", s1);

    return 0;
}