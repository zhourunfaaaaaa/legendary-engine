#include <bits/stdc++.h>
using namespace std;

// 暴力枚举，N < 10000 直接逐个检查

int main()
{
    int n, k;
    cin >> n >> k;

    set<int> bad;
    for (int i = 0; i < k; i++)
    {
        int d;
        cin >> d;
        bad.insert(d);
    }

    for (int ans = n; ; ans++)
    {
        int x = ans;
        bool ok = true;
        while (x > 0)
        {
            if (bad.count(x % 10))
            {
                ok = false;
                break;
            }
            x /= 10;
        }
        if (ok)
        {
            cout << ans << endl;
            return 0;
        }
    }
}
