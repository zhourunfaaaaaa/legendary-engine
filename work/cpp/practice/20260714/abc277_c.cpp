#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    // 双向建立图
    map<int, vector<int>> g;

    for (int i = 0; i < n; i++) {
        int a, b;
        cin >> a >> b;

        g[a].push_back(b);
        g[b].push_back(a);
    }

    set<int> visited;
    queue<int> q;

    // 需要用BFS遍历图
    visited.insert(1);
    q.push(1);

    while (!q.empty()) {
        int x = q.front();
        q.pop();

        for (int y : g[x]) {
            if (!visited.count(y)) {
                visited.insert(y);
                q.push(y);
            }
        }
    }

    cout << *visited.rbegin() << '\n';
    return 0;
}