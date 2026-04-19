#include <bits/stdc++.h>
using namespace std;

int n, k, ans = 0, num[25];

bool isPrime(int x) {
    if (x < 2) return false;
    for (int i = 2; i * i <= x; i++)
        if (x % i == 0) return false;
    return true;
}

// step: 已选个数, sum: 当前和, start: 下次搜索起点
void f(int step, int sum, int start) {
    if (step == k) {
        if (isPrime(sum)) ans++;
        return;
    }

    for (int i = start; i < n; i++) {
        // 使用 sum + num[i] 传参，利用值传递实现“自动回溯”
        f(step + 1, sum + num[i], i + 1);
    }
}

int main() {
    cin >> n >> k;
    for (int i = 0; i < n; i++) cin >> num[i];

    f(0, 0, 0);
    cout << ans;
    return 0;
}