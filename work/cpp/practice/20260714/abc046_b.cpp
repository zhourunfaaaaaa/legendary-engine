#include <bits/stdc++.h>
using namespace std;

long long qpow(int a, int b)
{
    long long res = 1;
    while (b > 0) {
        if (b & 1) {
            res = res * a;
        }
        a = a * a;
        b >>= 1;
    }
    return res;
}

int main()
{
    int n, k;
    cin >> n >> k;
    long long ans = qpow(k - 1, n - 1);
    cout << ans * k;

    return 0;
}