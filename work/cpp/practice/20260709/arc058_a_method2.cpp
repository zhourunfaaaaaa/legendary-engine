#include <bits/stdc++.h>
using namespace std;

// 贪心 + 数位构造
// next[d]: 最小可用数字 >= d，-1 表示没有
// 从左往右扫，能匹配就匹配，能取大就取大然后后面全填最小，不能就回溯

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

    vector<int> nxt(10, -1);
    int cur = -1;
    for (int d = 9; d >= 0; d--)
    {
        if (!bad.count(d))
            cur = d;
        nxt[d] = cur;
    }
    int lo = nxt[0]; // 最小可用数字，填位用

    string s = to_string(n);
    int len = s.size();

    int i = 0;
    while (i < len)
    {
        int d = s[i] - '0';

        if (nxt[d] == -1)
        {
            // 回溯：往左找第一个能改大的位
            while (--i >= 0)
                if (nxt[s[i] - '0' + 1] != -1)
                    break;

            if (i < 0)
                // 多一位，首位取最小非零可用
                cout << char('0' + nxt[1]) << string(len, '0' + lo) << endl;
            else
                cout << s.substr(0, i)
                     << char('0' + nxt[s[i] - '0' + 1])
                     << string(len - i - 1, '0' + lo) << endl;
            return 0;
        }
        else if (nxt[d] > d)
        {
            cout << s.substr(0, i)
                 << char('0' + nxt[d])
                 << string(len - i - 1, '0' + lo) << endl;
            return 0;
        }
        i++;
    }

    cout << s << endl; // N 本身全合法
}
