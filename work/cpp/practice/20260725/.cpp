#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long M;
    cin >> N >> M;

    vector<long long> A(N);
    long long total = 0;

    for (long long& value : A) {
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
    long long ok = 0;
    long long ng = *max_element(A.begin(), A.end());

    while (ng - ok > 1) {
        long long mid = ok + (ng - ok) / 2;

        long long cost = 0;
        for (long long value : A) {
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
