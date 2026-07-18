#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long X;
    cin >> N >> X;

    vector<long long> A(N);
    for (long long& x : A) {
        cin >> x;
    }

    long long answer = 0;

    for (int i = 1; i < N; ++i) {
        long long excess = max(0LL, A[i - 1] + A[i] - X);
        answer += excess;

        long long eatCurrent = min(A[i], excess);
        A[i] -= eatCurrent;
        A[i - 1] -= excess - eatCurrent;
    }

    cout << answer << '\n';
    return 0;
}
