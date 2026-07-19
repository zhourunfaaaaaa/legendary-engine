#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    cin >> N;

    int answer = 0;

    for (int i = 0; i < N; ++i) {
        int A, B;
        string S;
        cin >> A >> B >> S;

        if (S == "keep") {
            answer += B - A;
        }
    }

    cout << answer << '\n';
    return 0;
}
