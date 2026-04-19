#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> ret;

void dfs(int step) {
    if (step == n+1) {
        for (int i = 0; i < ret.size(); i++) {
            cout << ret[i] << " ";
        }
        cout << endl;
        return;
    }
    ret.push_back(step);
    dfs(step+1);

    ret.pop_back();
    dfs(step+1);

}

int main ()
{
    cin >> n;
    dfs(1);
    return 0;
}