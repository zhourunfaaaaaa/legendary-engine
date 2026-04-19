#include <iostream>
#include <cstdio>

using namespace std;

const int N = 15;
int n;
int s[N]; // 用来记录每个点的状态：0表示待考虑，1表示选择该数，2表示不选

void dfs(int x) {
    if (x == n) {
        for (int i = 0; i < n; i++) {
            if (s[i] == 1) {
                printf("%d ", i + 1);
            }
        }
        printf("\n");
        return;
    }

    s[x] = 2;
    dfs(x + 1); // 第一个分支，不选
    s[x] = 0;

    s[x] = 1;
    dfs(x + 1); // 第二个分支，选
    s[x] = 0;
}

int main() {
    cin >> n;
    dfs(0);
    return 0;
}