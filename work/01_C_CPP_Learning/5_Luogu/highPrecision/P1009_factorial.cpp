#include <bits/stdc++.h>

using namespace std;

int main()
{
    int n;
    cin >> n;

    vector<int> fact = {1};
    vector<int> sum = {0};

    // 高精度乘法
    for (int i = 1;i <= n;i++) {
        int carry = 0;
        for (int j = 0;j < fact.size();j++) {
            fact[j] = fact[j] * i + carry;
            carry = fact[j] / 10;
            fact[j] %= 10;
        }
        while (carry) {
            fact.push_back(carry%10);
            carry /= 10;
        }
        // 高精度加法
        carry = 0;
        int max_len = max(sum.size(),fact.size());
        sum.resize(max_len,0); // 对齐位数,不足补0
        for (int j = 0;j < max_len;j++) {
            sum[j] = sum[j] + fact[j] + carry;
            carry = sum[j] / 10;
            sum[j] %= 10;
        }
        while (carry) {
            sum.push_back(carry%10);
            carry /= 10;
        }
    }

    for (int i = sum.size()-1;i >= 0;i--) {
        cout << sum[i];
    }
    
    return 0;
}