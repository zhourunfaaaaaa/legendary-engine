#include <bits/stdc++.h>
using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int h, w;
    cin >> h >> w;

    vector<string> grid(h);
    for (string &row : grid) {
        cin >> row;
    }

    int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    int answer = 0;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (grid[i][j] != '#') {
                continue;
            }

            answer++;

            queue<pair<int, int>> q;
            q.push({i, j});
            grid[i][j] = '.';

            while (!q.empty()) {
                auto [x, y] = q.front();
                q.pop();

                for (int direction = 0; direction < 8; direction++) {
                    int nx = x + dx[direction];
                    int ny = y + dy[direction];

                    if (nx < 0 || nx >= h || ny < 0 || ny >= w) {
                        continue;
                    }

                    if (grid[nx][ny] != '#') {
                        continue;
                    }

                    grid[nx][ny] = '.';
                    q.push({nx, ny});
                }
            }
        }
    }

    cout << answer << '\n';
    return 0;
}