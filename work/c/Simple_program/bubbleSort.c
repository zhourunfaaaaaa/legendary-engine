#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int s[n];
    int i,j;
    for (i = 0;i < n;i++) {
        scanf("%d",&s[i]);
    }

    for (i = 0;i < n-1;i++) {
        for (j = 0;j < n;j++) {
            if (s[j] > s[j+1]) {
                int t = s[j];
                s[j] = s[j+1];
                s[j+1] = t;
            }
        }
    }

    for (i = 0;i < n;i++) {
        printf("%d ",s[i]);
    }
}