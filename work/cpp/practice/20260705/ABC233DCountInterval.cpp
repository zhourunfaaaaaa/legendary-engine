// 前缀和加哈希表，直接暴力查询复杂度是O(n2)，无法达到要求
// 前缀和好想，哈希表查询第一次见

#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    long long k;
    cin >> n >> k;
    
    vector<long long> pre(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        long long number;
        cin >> number;
        pre[i] = pre[i - 1] + number;
    }
    
    unordered_map<long long, long long> cnt;
    cnt[0] = 1;
    long long ans = 0;
    
    for (int i = 1; i <= n; i++) {
        ans += cnt[pre[i] - k]; // 先查询后插入保证不重复，若n不是有效的键，cnt[n] = 0；
        cnt[pre[i]]++;          // 同时查询和插入，由于l-1一定在r的左边，所以保证不遗漏
    }
    
    cout << ans << endl;
    return 0;
}