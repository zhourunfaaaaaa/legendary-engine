#include <bits/stdc++.h>
using namespace std;

struct Item {
    long long weight;
    int value;
};

int main()
{
    int n;
    long long w;
    cin >> n >> w;

    vector<Item> things(n);

    for (int i = 0; i < n; i++) {
        cin >> things[i].weight >> things[i].value;
    }

    // W 最大到 1e9，不能按重量开 dp
    // 但价值最多 100 * 1000 = 100000，所以改成按价值做 dp
    int total_value = 0;
    for (int i = 0; i < n; i++) {
        total_value += things[i].value;
    }

    // dp[v] 表示：达到总价值 v 所需要的最小重量
    // INF 表示这个价值当前还无法达到，不能初始化成 0
    const long long INF = 4e18;
    vector<long long> dp(total_value + 1, INF);
    dp[0] = 0; // 价值为 0 时，什么都不选，重量也是 0

    for (int i = 0; i < n; i++) {
        // 01 背包每个物品只能选一次，所以价值要倒序枚举
        // 如果正序，当前物品刚更新出的状态可能会被本轮再次使用
        for (int v = total_value; v >= things[i].value; v--) {
            // 不选当前物品：dp[v] 保持不变
            // 选当前物品：从 v - value 转移过来，并加上当前物品重量
            dp[v] = min(dp[v], dp[v - things[i].value] + things[i].weight);
        }
    }

    // 找最大的价值 v，只要达到它的最小重量不超过背包容量 w，就可行
    int ans = 0;
    for (int v = 0; v <= total_value; ++v) {
        if (dp[v] <= w) {
            ans = v;
        }
    }

    cout << ans << endl;

    return 0;
}
