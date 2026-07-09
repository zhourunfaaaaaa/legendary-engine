#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, l;
    cin >> n >> l;

    vector<string> s(n);
    for (int i = 0; i < n; i++) {
        cin >> s[i];
    }

    sort(s.begin(), s.end(), [](const string& a, const string& b) {
        return a + b < b + a;
    });

    for (auto& t : s) {
        cout << t;
    }
    cout << endl;

    return 0;
}