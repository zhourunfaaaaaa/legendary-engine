#include <bits/stdc++.h>

using namespace std;


const long long MOD = 998244353;

long long modpow(long long a, long long b) // 快速幂
{
    long long res = 1;
    while (b > 0) {
        if (b & 1) {
            res = res * a % MOD; // 对于任何可能超出范围的值都要取模，无论取模多少次都不会影响最终结果
        }
        a = a * a % MOD;
        b >>= 1;
    }

    return res;
}

int main()
{
    int n, p;
    cin >> n >> p;
    vector<long long> dp(n+2);
    dp[0] = 0;
    dp[1] = 1; // 要求在模意义下求期望，那么概率也要换算为模意义下的
    long long prob2 = p * modpow(100, MOD - 2) % MOD;
    long long prob1 = (100 - p) * modpow(100, MOD - 2) % MOD;

    for (int i = 2; i <= n; i++) {
        dp[i] = (1 + prob1 * dp[i-1] + prob2 * dp[i-2]) % MOD ; // +1 是因为从 i-1 或 i-2 到 i 还需要一次攻击，dp[i] 代表的是打 i 血要的攻击次数
    }

    cout << dp[n] << endl;

    return 0;
}
