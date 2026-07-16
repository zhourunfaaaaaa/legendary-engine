#include <bits/stdc++.h>
using namespace std;

int main() {
    string S;
    cin >> S;

    int answer = 0;

    for (int i = 1; i < (int)S.size(); ++i) {
        if (S[i] != S[i - 1]) {
            ++answer;
        }
    }

    cout << answer << '\n';
    return 0;
}