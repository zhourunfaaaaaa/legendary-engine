#include <bits/stdc++.h>
using namespace std;

bool ask(int i, int j)
{
    cout << "? " << i << " " << j << endl;

    string response;
    cin >> response;

    return response == "Yes";
}

int main()
{
    int n;
    cin >> n;
    int r = 1;
    int ans = 0;
    // 双指针
    for (int l = 1; l <= n; l++) {
        if (l > r) {
            r = l;
        }
        
        while (r < n) {
            if (ask(l, r + 1)) {
                r++;
            } else {
                break;
            }
        }
        
        ans += r - l;
    }

    cout << "! " << ans << endl;

    return 0;
}