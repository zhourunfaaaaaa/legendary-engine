#include <iostream>
#include <vector>

using namespace std;

void dfs(int index, int n, vector<int> current) {
    if (index > n) {
        for(int i = 0; i < current.size(); i++) {
            cout << current[i] << " ";
        }
        cout << endl;
        return;
    }

    // 不选当前数
    dfs(index + 1, n, current);

    // 选当前数
    current.push_back(index);
    dfs(index + 1, n, current);
}

int main() {
    int n;
    cin >> n;

    vector<int> current;
    dfs(1, n, current);

    return 0;
}