#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int Q;
    cin >> Q;

    queue<pair<int64, int64>> balls;

    while (Q--) {
        int type;
        cin >> type;

        if (type == 1) {
            int64 x, c;
            cin >> x >> c;
            balls.emplace(x, c);
        } else {
            int64 c;
            cin >> c;

            int64 answer = 0;

            while (c > 0) {
                int64 x = balls.front().first;
                int64 count = balls.front().second;
                int64 take = min(c, count);

                answer += x * take;
                c -= take;
                balls.front().second -= take;

                if (balls.front().second == 0) {
                    balls.pop();
                }
            }

            cout << answer << '\n';
        }
    }

    return 0;
}