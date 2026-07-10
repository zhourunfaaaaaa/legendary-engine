#include <bits/stdc++.h>
using namespace std;

const long long MOD = 1000000007;

long long qpow(long long a, long long b)
{
    long long res = 1;
    while (b > 0) {
        if (b & 1) {
            res = res * a % MOD;
        }
        a = a * a % MOD;
        b >>= 1;
    }
    return res;
}

long long C(int n, int k, vector<long long>& fac, vector<long long>& invfac)
{
    if (k < 0 || k > n) {
        return 0;
    }
    return fac[n] * invfac[k] % MOD * invfac[n - k] % MOD;
}

int main()
{
    int H, W, A, B;
    cin >> H >> W >> A >> B;

    int maxN = H + W + 5;
    vector<long long> fac(maxN), invfac(maxN);

    fac[0] = 1;
    for (int i = 1; i < maxN; i++) {
        fac[i] = fac[i - 1] * i % MOD;
    }

    invfac[maxN - 1] = qpow(fac[maxN - 1], MOD - 2);
    for (int i = maxN - 2; i >= 0; i--) {
        invfac[i] = invfac[i + 1] * (i + 1) % MOD;
    }

    long long ans = 0;

    /*
    路径拆分思路：

    黑色区域在左下角：
    最下面 A 行、最左边 B 列不能走。

    所以第 H - A 行是黑色区域上方的最后一行，
    第 H - A + 1 行是黑色区域所在的第一行。

    一条合法路径从左上角 (1,1) 走到右下角 (H,W)，
    中途一定要从上方区域进入下面 A 行。

    因为只能向右或向下走，所以它进入下面区域的那一步一定长这样：
    (H-A, i) -> (H-A+1, i)

    这里的 i 表示从第 i 列往下跨。

    但是第 H-A+1 行的前 B 列属于黑色区域，
    也就是这些格子不能走：
    (H-A+1, 1), (H-A+1, 2), ..., (H-A+1, B)

    所以 i 不能小于等于 B，只能从 B + 1 到 W 枚举。

    因此每条合法路径都可以唯一拆成：
    (1,1) -> (H-A, i) -> (H-A+1, i) -> (H,W)

    对于固定的 i：

    上半段：
    (1,1) -> (H-A, i)
    需要向下 H-A-1 步，向右 i-1 步。
    方案数是：
    C((H-A-1) + (i-1), H-A-1)

    下半段：
    (H-A+1, i) -> (H,W)
    需要向下 A-1 步，向右 W-i 步。
    方案数是：
    C((A-1) + (W-i), A-1)

    固定 i 的总方案数就是：
    上半段方案数 * 下半段方案数

    最后把所有 i 的情况加起来。
    */

    for (int i = B + 1; i <= W; i++) {
        long long upper = C(H - A + i - 2, H - A - 1, fac, invfac);
        long long lower = C(A + W - i - 1, A - 1, fac, invfac);
        ans = (ans + upper * lower) % MOD;
    }

    cout << ans << endl;

    return 0;
}
