#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;

    vector<int> A(N), B(N - 1);
    for (int& x : A) {
        cin >> x;
    }
    for (int& x : B) {
        cin >> x;
    }

    int answer = N;

    for (int firstParity = 0; firstParity < 2; ++firstParity) {
        int parity = firstParity;
        int operations = 0;

        for (int i = 0; i < N; ++i) {
            if (A[i] != parity) {
                ++operations;
            }

            if (i < N - 1) {
                parity ^= B[i];
            }
        }

        answer = min(answer, operations);
    }

    cout << answer << '\n';
    return 0;
}
