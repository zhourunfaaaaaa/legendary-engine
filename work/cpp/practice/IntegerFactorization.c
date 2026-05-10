#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int m = n;
    int i = 1;
    while (m > 0) {
        m /= 10;
        i *= 10;
    }
    i /= 10;

    while (n > 0) {
        printf("%d ",n/i);
        n %= i;
        i /= 10;
    }
    return 0;
}