#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    vector<string> s(n);

    // 读入的是int的话就要to_string转为string
    for (int i = 0; i < n; i++) {
        cin >> s[i];
    }

    sort(s.begin(), s.end(), [](string a, string b) {
        return a + b > b + a;
    });

    if (s[0] == "0") {
        cout << 0;
        return 0;
    }

    for (string x : s) {
        cout << x;
    }

    return 0;
}