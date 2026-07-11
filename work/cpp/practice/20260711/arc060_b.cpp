#include <bits/stdc++.h>
using namespace std;

long long digitSum(long long n, long long b)
{
    long long sum = 0;

    while (n > 0) {
        // 余数是当前最低位，除以b后去掉这一位
        sum += n % b;
        n /= b;
    }

    return sum;
}

int main()
{
    long long n, s;
    cin >> n >> s;

    // 数位和不可能大于n
    if (s > n) {
        cout << -1 << endl;
        return 0;
    }

    // b大于n时，n只有一位，最小的b是n+1
    if (s == n) {
        cout << n + 1 << endl;
        return 0;
    }

    long long ans = LLONG_MAX;

    // 小进制只有根号n个，直接逐个检查
    for (long long b = 2; b * b <= n; b++) {
        if (digitSum(n, b) == s) {
            ans = min(ans, b);
        }
    }

    // 大进制下n最多两位：n = p * b + q，且p + q = s
    // 所以n - s = p * (b - 1)，枚举较小的高位p来算b
    for (long long p = 1; p * p <= n; p++) {
        if ((n - s) % p != 0) {
            continue;
        }

        long long b = (n - s) / p + 1;

        // 用算出的b重新检查，防止数位不合法
        if (b >= 2 && b > n / b) {
            if (digitSum(n, b) == s) {
                ans = min(ans, b);
            }
        }
    }

    if (ans == LLONG_MAX) {
        cout << -1 << endl;
    } else {
        cout << ans << endl;
    }

    return 0;
}