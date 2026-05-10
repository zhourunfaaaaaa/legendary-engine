#include <stdio.h>

int main()
{
    int x = 0xDEF;
    printf("%4d,%4o,%4x\n",x,x,x);

    x = 015;
    int y = 0x15;
    printf("%4o,%4x\n",x,y);//八进制 十六进制
    printf("%4x,%4d\n",x,y);//十六进制 十进制
    printf("%4d,%4o\n",x,y);//十进制 八进制

    char c1 = 'a',c2 = 'b',c3 = 'c',c4 = '\101',c5 = '\116';
    //\101是八进制转义序列，表示该八进制数值对应的ASXII字符
    printf("abc\tde\bh\rA\tg\n");//\b是光标回退一格,\r是光标回到首格
    printf("a%cb%c\tc%c\tabc\n",c1,c2,c3);
    //\t不是四个空格，而是它的宽度取决于终端的制表位设置，通常是 8 列一个制表位
    printf("\t\b%c%c",c4,c5); 
    
    return 0;
}