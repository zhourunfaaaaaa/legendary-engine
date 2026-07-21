#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M;
    cin >> N >> M;

    vector<vector<bool>> adjacent(N + 1, vector<bool>(N + 1, false));

    for (int i = 0; i < M; ++i) {
        vector<int> A(N);
        for (int& x : A) {
            cin >> x;
        }

        for (int j = 0; j < N - 1; ++j) {
            adjacent[A[j]][A[j + 1]] = true;
            adjacent[A[j + 1]][A[j]] = true;
        }
    }

    int answer = 0;

    for (int i = 1; i <= N; ++i) {
        for (int j = i + 1; j <= N; ++j) {
            if (!adjacent[i][j]) {
                ++answer;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}
