#include <bits/stdc++.h>
using namespace std;

int main()
{
    string w;
    cin >> w;

    unordered_map<char, int> cnt;

    for (char c : w) {
        cnt[c]++;
    }

    for (auto it : cnt) {
        if (it.second % 2 == 1) {
            cout << "No" << endl;
            return 0;
        }
    }

    cout << "Yes" << endl;

    return 0;
}