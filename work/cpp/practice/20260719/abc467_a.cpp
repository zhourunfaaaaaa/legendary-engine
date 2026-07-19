#include <bits/stdc++.h>
using namespace std;

int main() {
    int H, W;
    cin >> H >> W;

    if (W * 10000 >= 25 * H * H) {
        cout << "Yes\n";
    } else {
        cout << "No\n";
    }

    return 0;
}
