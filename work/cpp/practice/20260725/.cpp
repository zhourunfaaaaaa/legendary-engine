#include <bits/stdc++.h>
using namespace std;

using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    ll M;
    cin >> N >> M;

    vector<ll> A(N);
    ll total = 0;

    for (ll& value : A) {
        cin >> value;
        total += value;
    }

    // 上限可以无限增大
    if (total <= M) {
        cout << "infinite\n";
        return 0;
    }

    // ok：一定可行
    // ng：一定不可行
    ll ok = 0;
    ll ng = *max_element(A.begin(), A.end());

    while (ng - ok > 1) {
        ll mid = ok + (ng - ok) / 2;

        ll cost = 0;
        for (ll value : A) {
            cost += min(value, mid);
        }

        if (cost <= M) {
            ok = mid;
        } else {
            ng = mid;
        }
    }

    cout << ok << '\n';
    return 0;
}