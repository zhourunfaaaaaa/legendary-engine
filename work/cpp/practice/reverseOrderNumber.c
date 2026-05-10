#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int sum = 0;
    while (n > 0) {
        int i = n % 10;
        n /= 10;
        sum = sum*10 + i;
    }
    printf("%d",sum);
}