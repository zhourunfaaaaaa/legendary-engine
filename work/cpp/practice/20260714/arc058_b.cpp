#include <bits/stdc++.h>
using namespace std;

int main()
{
    string s;
    cin >> s;
    int l = s.length();
    int g_cnt = 0;
    int p_cnt = 0;
    int ans = 0;
    for (int i = 0; i < l; i++) {
        if (s[i] == 'p') {
            if (g_cnt > p_cnt) {
                p_cnt++;
            } else {
                g_cnt++;
                ans--;
            }
        } else {
            if (g_cnt > p_cnt) {
                ans++;
                p_cnt++;
            } else {
                g_cnt++;
            }
        }
    }
    cout << ans << endl;

    return 0;
}