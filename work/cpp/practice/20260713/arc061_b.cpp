#include <bits/stdc++.h>
using namespace std;

int main()
{
    long long h, w;
    int n;
    cin >> h >> w >> n;

    map<pair<long long, long long>, int> cnt;
    for (int i = 0; i < n; i++) {
        long long a, b;
        cin >> a >> b;

        // 可以写成更紧凑的for (long long x = max(1LL, a - 2); x <= min(a, h - 2); x++)
        // 同时根据 a - 2 <= x <= a(点附近的三个3*3); 1 <= x <= h - 2(3*3不超出范围)
        for (long long i = -2; i <= 0; i++) {
            long long x = a + i;

            if (x < 1 || x > h - 2) {
                continue;
            }

            for (long long j = -2; j <= 0; j++) {
                long long y = b + j;

                if (y < 1 || y > w - 2) {
                    continue;
                }

                cnt[{x, y}]++;
            }
        }
    }

    vector<int> ans(10, 0);

    for (auto p : cnt) {
        ans[p.second]++;
    }

    // 这也是关键一步
    ans[0] = (h - 2) * (w - 2) - cnt.size();

    for (int i = 0; i < 10; i++) {
        cout << ans[i] << endl;
    }

    return 0;
}