#include <stdio.h>

int main() {
    int a = 0x12345678;
    char* p = (char*)&a;  // 指向 a 的第一个字节
    
    printf("地址 %p 存放的字节: %x\n", p, *p);
    printf("地址 %p 存放的字节: %x\n", p+1, *(p+1));
    printf("地址 %p 存放的字节: %x\n", p+2, *(p+2));
    printf("地址 %p 存放的字节: %x\n", p+3, *(p+3));
    
    return 0;
}