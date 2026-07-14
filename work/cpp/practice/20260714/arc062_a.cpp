#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    // 当前满足前面所有速报的最小票数
    long long t = 1;
    long long a = 1;

    for (int i = 0; i < n; i++) {
        long long T, A;
        cin >> T >> A;

        // 当前票数必须是T * k和A * k，且不能比之前少
        // (t + T - 1) / T 是经典的向上取整写法
        long long k1 = (t + T - 1) / T;
        long long k2 = (a + A - 1) / A;
        // 同时满足两个人所需的最小倍数
        long long k = max(k1, k2);
        
        t = T * k;
        a = A * k;
    }
    
    cout << a + t << endl;

    return 0;
}
