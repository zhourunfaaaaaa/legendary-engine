#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    long long k;
    cin >> n >> k;
    vector<long long> pre(n+1);
    pre[0] = 0;
    for (int i = 1; i <= n; i++) {
        int x;
        cin >> x;
        pre[i] = pre[i-1] + x;
    }
    
    auto it1 = lower_bound(pre.begin(), pre.end(), k);
    long long ans = 0;
    for ( ; it1 != pre.end(); it1++) {
        auto it2 = upper_bound(pre.begin(), pre.end(), *it1 - k);
        if (it2 <= it1) {
            ans += it2 - pre.begin();
        }
    }

    cout << ans << endl;
    return 0;
}