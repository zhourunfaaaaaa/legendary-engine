// 看似算法，实则数学题，有时候不要想的太复杂，可能存在容易发现的数学规律
#include <bits/stdc++.h>
using namespace std;

int main() {
    long long sum = 0;
    int n = 0;
    int x;

    while (cin >> x) {
        sum += x;
        n++;
    }

    long long count = 1LL << (n - 1);
    
    cout << sum * count << endl;
    
    return 0;
}