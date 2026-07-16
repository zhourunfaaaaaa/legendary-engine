#include <bits/stdc++.h>
using namespace std;

int main() {
    int N;
    long long T;
    cin >> N >> T;

    vector<long long> A(N);
    for (long long& x : A) {
        cin >> x;
    }

    long long minPrice = A[0];
    long long maxProfit = 0;
    int answer = 0;

    for (int i = 1; i < N; ++i) {
        long long profit = A[i] - minPrice;

        if (profit > maxProfit) {
            maxProfit = profit;
            answer = 1;
        } else if (profit == maxProfit) {
            ++answer;
        }

        minPrice = min(minPrice, A[i]);
    }

    cout << answer << '\n';
    return 0;
}