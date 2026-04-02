#include <bits/stdc++.h>
using namespace std;

int main() {
    int n, m;
    cin >> n >> m;
    
    vector<vector<char> > boom(n, vector<char>(m));
    
    // 读入地图
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cin >> boom[i][j];
        }
    }
    
    // 8个方向：上、下、左、右、左上、右上、左下、右下(图表常用方法:方向数组)
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    // 遍历每个格子
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (boom[i][j] == '*') {
                cout << '*';  // 地雷直接输出
                continue;
            }
            
            // 统计周围地雷数
            int cnt = 0;
            for (int k = 0; k < 8; k++) {// 八个方向各一次，非常好的思路
                int ni = i + dx[k];
                int nj = j + dy[k];
                // 检查边界
                if (ni >= 0 && ni < n && nj >= 0 && nj < m) {
                    if (boom[ni][nj] == '*') {
                        cnt++;
                    }
                }
            }
            cout << cnt;  // 输出数字
        }
        cout << endl;
    }
    
    return 0;
}