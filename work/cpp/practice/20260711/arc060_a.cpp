#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, a;
    cin >> n >> a;

    vector<int> x(n);
    int sum = 0;

    for (int i = 0; i < n; i++) {
        cin >> x[i];
        sum += x[i];
    }

    vector<vector<long long>> dp(n + 1, vector<long long>(sum + 1, 0));

    dp[0][0] = 1;

    for (int i = 0; i < n; i++) {
        for (int k = i + 1; k >= 1; k--) {
            for (int s = sum; s >= x[i]; s--) {
                dp[k][s] += dp[k - 1][s - x[i]];
            }
        }
    }

    long long ans = 0;

    // 每一组满足的方法之和，比如dp[1][8] + dp[2][16] + dp[3][24] + ...
    for (int k = 1; k <= n; k++) {
        ans += dp[k][a * k];
    }

    cout << ans << endl;

    return 0;
}