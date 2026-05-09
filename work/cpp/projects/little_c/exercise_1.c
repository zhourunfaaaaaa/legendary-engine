#include <stdio.h>

int main()
{
    int t1,t2;
    scanf("%d %d",&t1,&t2);
    int a = t1 / 100;
    int b = t1 % 100;
    int c = 60 * a + b + t2;
    int d = c / 60;
    int e = c % 60;

    printf("%d",d * 100 + e);
    return 0;
}