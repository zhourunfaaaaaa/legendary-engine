#include <bits/stdc++.h>

using namespace std;

vector<int> multiply(vector<int> &a,vector<int> &b) {
    vector<int> rst(a.size()+b.size(), 0);

    for (size_t i = 0; i < a.size(); i++) {
        for (size_t j = 0; j < b.size(); j++) {
            rst[i+j] += a[i] * b[j];
            rst[i+j+1] += rst[i+j] / 10;
            rst[i+j] %= 10;
        }
    }
    while (rst.size() > 1 && rst.back() == 0) {
        rst.pop_back();
    }
    return rst;
}

int main()
{
    vector<int> a = {2,1};
    vector<int> b = {2,1};
    vector<int> rst = multiply(a,b);
    
    for (int i = rst.size() - 1; i >= 0; i--) {
        cout << rst[i];
    }

    return 0;
}