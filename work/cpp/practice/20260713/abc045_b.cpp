#include <bits/stdc++.h>
using namespace std;

int main()
{
    string s[3];
    cin >> s[0] >> s[1] >> s[2];

    int pos[3] = {0, 0, 0};
    int now = 0;

    while (true) {
        if (pos[now] == s[now].length()) {
            cout << char('A' + now) << endl;
            break;
        }

        char card = s[now][pos[now]];
        pos[now]++;

        now = card - 'a';
    }

    return 0;
}