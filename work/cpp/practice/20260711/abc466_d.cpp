#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, m;
    cin >> n >> m;
    vector<int> r(m + 1);
    vector<int> c(m + 1);

    for (int i = 1; i <= m; i++) {
        cin >> r[i] >> c[i];
    }

    // 用逆序思维，只有后面的操作没有清理前面的行和列时，前面棋子才能保留
    vector<bool> used_r(m + 1, false);
    vector<bool> used_c(m + 1, false);

    int ans = 0;
    for (int i = m; i >= 1; i--) {
        if (!used_r[r[i]] && !used_c[c[i]]) {
            ans++;
        }
    
        used_r[r[i]] = true;
        used_c[c[i]] = true;
    }

    cout << ans << endl;

    return 0;
}