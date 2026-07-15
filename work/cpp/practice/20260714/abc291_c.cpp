#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    string s;
    cin >> n >> s;

    int x = 0, y = 0;
    map<pair<int, int>, int> used;
    used[{0, 0}] = 1;
    for (int i = 0; i < n; i++) {
        if (s[i] == 'R') {
            x++;
        } else if (s[i] == 'L') {
            x--;
        } else if (s[i] == 'U') {
            y++;
        } else if (s[i] == 'D') {
            y--;
        }
        if (used[{x, y}]) {
            cout << "Yes" << endl;

            return 0;
        }

        used[{x, y}] = 1;
    }
    
    cout << "No" << endl;

    return 0;
}