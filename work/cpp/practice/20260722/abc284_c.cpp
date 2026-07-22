#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, m;
    cin >> n >> m;

    vector<vector<int>> graph(n + 1);

    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;

        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<bool> visited(n + 1, false);
    int answer = 0;

    // 遍历每个点
    for (int start = 1; start <= n; start++) {
        if (visited[start]) {
            continue;
        }

        // 找到一个新的连通分量
        answer++;

        queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            for (int next : graph[current]) {
                if (visited[next]) {
                    continue;
                }

                visited[next] = true;
                q.push(next);
            }
        }
    }

    cout << answer << '\n';
    return 0;
}