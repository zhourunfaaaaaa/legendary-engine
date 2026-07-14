#include <bits/stdc++.h>
using namespace std;

int main()
{
    string s;
    cin >> s;

    string ans;

    // 简单的字符串操作题目
    for (char c : s) {
        if (c == 'B') {
            if (!ans.empty()) {
                ans.pop_back();
            }
        } else {
            ans.push_back(c);
        }
    }

    cout << ans << endl;

    return 0;
}