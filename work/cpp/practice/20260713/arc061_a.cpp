#include <bits/stdc++.h>
using namespace std;

int main()
{
    string s;
    cin >> s;

    int n = s.length();
    long long ans = 0;

    for (int mask = 0; mask < (1 << (n - 1)); mask++) {
        long long number = 0;

        for (int i = 0; i < n; i++) {
            number = number * 10 + (s[i] - '0');

            // 检查第i位是否为1，若不是1则不放加号，若不放加号则要用i == n - 1判断
            if (i == n - 1 || mask & (1 << i)) {
                ans += number;
                number = 0;
            }
        }
    }

    cout << ans << endl;

    return 0;
}