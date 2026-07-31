#include <bits/stdc++.h>
using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n1, n2, n3;
    cin >> n1 >> n2 >> n3;

    vector<long long> A(n1), B(n2), C(n3);

    for (long long &x : A) {
        cin >> x;
    }

    for (long long &x : B) {
        cin >> x;
    }

    for (long long &x : C) {
        cin >> x;
    }

    vector<long long> sum;

    for (long long a : A) {
        for (long long b : B) {
            sum.push_back(a + b);
        }
    }

    sort(sum.begin(), sum.end());
    sum.erase(unique(sum.begin(), sum.end()), sum.end());

    sort(C.begin(), C.end());

    if (sum == C) {
        cout << "YES\n";
    } else {
        cout << "NO\n";
    }

    return 0;
}