#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, m;
    cin >> n >> m;
    
    vector<long long> dp(m + 1, -1);
    dp[1] = 0;

    /*
    核心思路：按行处理 + 按墙切连续区间。

    因为只能向左、向右、向下走，不能向上走，
    所以一旦从第 i 行走到第 i+1 行，就再也回不到第 i 行。
    因此可以一行一行地处理。

    dp[j] 的意思：
    处理完上一行后，如果站在第 j 列，最多已经收集多少花瓣。

    dp[j] = -1 表示：
    处理完上一行后，不可能站在第 j 列。

    初始条件：
    dp[1] = 0
    可以理解为：还没进入地图前，人在第 1 行上方的第 1 列，
    可以从这里进入第 1 行，当前已经收集 0 个花瓣。

    对当前第 i 行，先按墙 -1 切成若干个连续非墙区间。
    例如当前行是：
    2 3 -1 5 7 1

    会被切成两段：
    [1,2]，花瓣和 sum = 2 + 3 = 5
    [4,6]，花瓣和 sum = 5 + 7 + 1 = 13

    对于一段 [l,r]：
    只需要看上一行的 dp[l] 到 dp[r] 里有没有可达位置。

    如果 best = max(dp[l], dp[l+1], ..., dp[r]) 是 -1，
    说明上一行没有任何一列可以往下进入这一段，这一段走不到。

    如果 best != -1，
    说明可以从上一行某一列往下进入这一段。
    因为这一段中间没有墙，进入后可以左右走，
    所以这一段里的所有格子都能走到，这一段的所有花瓣也都能收集。

    于是对所有 k in [l,r]：
    ndp[k] = best + sum

    具体例子：
    假设上一行处理完以后：
    列:    1   2   3   4   5   6
    dp:   -1  10  -1  -1  20  -1

    当前行：
    格子:  2   3  -1   5   7   1

    当前行分成两段：
    [1,2] 和 [4,6]

    处理 [1,2]：
    best = max(dp[1], dp[2]) = max(-1, 10) = 10
    sum = 2 + 3 = 5
    所以 ndp[1] = 15, ndp[2] = 15

    处理 [4,6]：
    best = max(dp[4], dp[5], dp[6]) = max(-1, 20, -1) = 20
    sum = 5 + 7 + 1 = 13
    所以 ndp[4] = 33, ndp[5] = 33, ndp[6] = 33

    如果某一段对应的 dp[l..r] 全是 -1，
    那一段就没有入口，保持不可达。

    最后 dp[m] 就是处理完整张图后，能否站在右下角。
    如果 dp[m] 是 -1，说明走不到终点。
    否则 dp[m] 就是最多能收集的花瓣数。
    */

    for (int i = 1; i <= n; i++) {
        vector<int> a(m + 1);

        for (int j = 1; j <= m; j++) {
            cin >> a[j];
        }

        vector<long long> ndp(m + 1, -1);
        
        int j = 1;
        while (j <= m) {
            if (a[j] == -1) {
                j++;
                continue;
            }

            int l = j;
            long long sum = 0;
            long long best = -1;

            while (j <= m && a[j] != -1) {
                sum += a[j];
                best = max(best, dp[j]);
                j++;
            }

            int r = j - 1;
            
            if (best != -1) {
                for (int k = l; k <= r; k++) {
                    ndp[k] = sum + best;
                }
            }
        }

        dp = ndp;
    }

    cout << dp[m] << endl;

    return 0;
}
