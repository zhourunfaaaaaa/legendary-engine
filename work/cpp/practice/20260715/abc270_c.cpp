#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, x, y;
    cin >> n >> x >> y;

    vector<vector<int>> graph(n + 1);

    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;

        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<int> parent(n + 1, -1);
    queue<int> q;

    parent[x] = 0;
    q.push(x);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        if (current == y) {
            break;
        }

        for (int next : graph[current]) {
            if (parent[next] != -1) {
                continue;
            }

            parent[next] = current;
            q.push(next);
        }
    }

    vector<int> path;

    for (int current = y; current != 0; current = parent[current]) {
        path.push_back(current);
    }

    reverse(path.begin(), path.end());

    for (int i = 0; i < static_cast<int>(path.size()); i++) {
        if (i > 0) {
            cout << ' ';
        }
        cout << path[i];
    }

    cout << '\n';
    return 0;
}