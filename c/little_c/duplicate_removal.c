#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int a[n];
    int b[n];
    int i,j;
    for (i = 0;i < n;i++) {
        scanf("%d",&a[i]);
        b[i] = 0;
    }

    for (i = 0,j = 0;i < n;) {
        b[j] = a[i];
        j++;
        while (b[j-1] == a[i] && i < n) {
            i++;
        }
    }

    for (i = 0;i < n;i++) {
        printf("%d",b[i]);
    }
    return 0;
}