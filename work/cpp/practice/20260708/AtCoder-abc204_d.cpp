#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;
    vector<int> t(n);
    int sum = 0;
    for (int i = 0; i < n; i++) {
        cin >> t[i];
        sum += t[i];
    }

    // 定义dp[t]为是否能凑出时间t，规则：若t能凑出来，则t + t[i]也能
    vector<int> dp(sum + 1, 0);
    dp[0] = 1;
    for (int i = 0; i < n; i++) {
        for (int j = sum; j >= t[i]; j--) {
            if (dp[j - t[i]]) {
                dp[j] = 1;
            }
        }
    }

    int ans = sum;
    for (int j = 0; j <= sum; j++) {
        if (dp[j]) {
            ans = min(ans, max(j, sum - j));
        }
    }

    cout << ans << endl;

    return 0;
}