#include <stdio.h>

int main()
{
    int n,m,hx,hy;
    scanf("%d %d %d %d",&n,&m,&hx,&hy);

    long long dp[25][25] = {0};
    int block[25][25] = {0};

    int dx[] = {1,2,-1,-2,1,2,-1,-2};
    int dy[] = {2,1,-2,-1,-2,-1,2,1};

    block[hx][hy] = 1;
    int i,j;
    for (i = 0;i < 8;i++) {
        int nx = hx + dx[i];
        int ny = hy + dy[i];
        if (nx <= n && nx >= 0 && ny <= m && ny >= 0) {
            block[nx][ny] = 1;
        }
    }

    if (block[0][0] == 1) {
        printf("o\n");
        return 0;
    }

    dp[0][0] = 1;
    for (i = 0;i <= n;i++) {
        for (j = 0;j <= m;j++) {
            if (block[i][j]) {
                continue;
            }
            if (i > 0) {
                dp[i][j] += dp[i - 1][j];
            }
            if (j > 0) {
                dp[i][j] += dp[i][j - 1];
            }
        }
    }

    printf("%d\n",dp[n][m]);
    return 0;
}