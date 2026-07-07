#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    vector<int> a(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
    }

    sort(a.begin(), a.end());

    int q;
    cin >> q;

    while (q--) {
        int b;
        cin >> b;

        auto it = lower_bound(a.begin(), a.end(), b); // lower_bound 经典二分

        int ans = INT_MAX;

        if (it != a.end()) {
            ans = min(ans, abs(*it - b));
        }

        if (it != a.begin()) {
            --it;
            ans = min(ans, abs(*it - b));
        }

        cout << ans << '\n';
    }

    return 0;
}
