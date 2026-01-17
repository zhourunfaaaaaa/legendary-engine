#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int i,j,k;
    int s[n][n];
    for (i = 0;i < n;i++) {
        for (j = 0;j < n;j++) {
            s[i][j] = 0;
        }
    }
    int x[n];
    int y[n];
    
    int a,b,c;
    int cnt = 0;
    while (1) {
        scanf("%d %d %d",&a,&b,&c);
        if (a == 0 && b == 0 && c== 0) {
            break;
        } else {
            x[cnt] = a;
            y[cnt] = b;
            s[a-1][b-1] = c;
        }
        cnt++;
    }

    cnt--;
    int sum[cnt];
    for (i = 0;i < cnt;i++) {
        sum[i] = 0;
        int record = y[cnt]-1;
        for (j = x[cnt]-1;j < n;j++) {
            for (k = record;k < n;k++) {
                sum[i] += s[j][k];
                s[j]
                if (s[j][k] != 0) {
                    record = k;
                }
            }
        }
    }

}