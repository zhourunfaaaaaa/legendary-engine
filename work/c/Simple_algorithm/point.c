#include <stdio.h>

int main()
{
    int xa, ya, xb, yb, xc, yc;
    
    printf("请输入三个点的坐标(x,y格式): ");
    scanf("%d,%d %d,%d %d,%d", &xa, &ya, &xb, &yb, &xc, &yc);
    
    int area = (xb - xa) * (yc - ya) - (xc - xa) * (yb - ya);
    
    if (area == 0) {
        printf("a,b,c在一条直线上。\n");
    } else {
        printf("a,b,c不在一条直线上。\n");
    }

    return 0;
}