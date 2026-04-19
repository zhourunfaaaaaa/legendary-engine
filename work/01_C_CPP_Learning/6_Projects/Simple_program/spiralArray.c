#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int a[n][n];
    int i,j = n-1;
    int s = 0,t = n-1,f = 0,cnt = 1;
    while (cnt <= n*n) {
        if (f%4 == 0) {
            for (i = s,j = s;j <= t;j++) {
                a[i][j] = cnt++;
            }
            f++;
        } else if (f%4 == 1) {
            for (i++,j--;i <= t;i++) {
                a[i][j] = cnt++;
            }
            f++;
        } else if (f%4 == 2) {
            for (i--,j--;j >= s;j--) {
                a[i][j] = cnt++;
            }
            f++;
            s++;
        } else if (f%4 == 3) {
            for (j++,i--;i >= s;i--) {
                a[i][j] = cnt++;
            }
            f++;
            t--;
        }
    }
    for (i = 0;i < n;i++) {
        for (j = 0;j < n;j++) {
            printf("%03d",a[i][j]);
            if (j != n-1) {
                printf(" ");
            }
        }
        printf("\n");
    }

    return 0;
}