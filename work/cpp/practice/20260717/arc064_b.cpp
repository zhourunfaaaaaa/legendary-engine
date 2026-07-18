#include <bits/stdc++.h>
using namespace std;

int main() {
    string S;
    cin >> S;

    bool sameEnds = S.front() == S.back();
    bool oddLength = S.size() % 2 == 1;

    // 终局长度的奇偶性由首尾字符是否相同决定
    if (sameEnds == oddLength) {
        cout << "Second\n";
    } else {
        cout << "First\n";
    }

    return 0;
}
