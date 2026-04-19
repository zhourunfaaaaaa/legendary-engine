#include <bits/stdc++.h>

using namespace std;

vector<vector<int>> rst;
vector<int> path;
vector<bool> used;

// 全排序和子集枚举一个最大的区别就是在dfs内多了一个for循环，因为要选够n个数字。
void dfs(vector<int> &nums) {
    if (path.size() == nums.size()) {
        rst.push_back(path);
        return;
    }

    for (int i = 0; i < nums.size(); i++) {
        if (used[i]) {
            continue;
        }

        used[i] = true;
        path.push_back(nums[i]);
        
        dfs(nums);// 选下一个数(递归)

        path.pop_back();// 把数放回去(回溯)
        used[i] = false;
    }
} 

int main()
{
    int n;
    cin >> n;
    vector<int> nums(n);
    for (int i = 0; i < n; i++) {
        nums[i] = i+1;
    }
    used = vector<bool>(nums.size(),false);
    dfs(nums);

    for (auto &p : rst) {
        for (int q : p) {
            cout << q << " ";
        }
        cout << endl;
    }
}