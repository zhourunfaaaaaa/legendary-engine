#include <bits/stdc++.h>
using namespace std;

int main() {
    long long A, B, X;
    cin >> A >> B >> X;

    long long answer = B / X;

    if (A == 0) {
        ++answer;
    } else {
        answer -= (A - 1) / X;
    }

    cout << answer << '\n';
    return 0;
}
