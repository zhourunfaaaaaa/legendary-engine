#include <bits/stdc++.h>
using namespace std;

int main() {
    int N, M, H, K;
    cin >> N >> M >> H >> K;

    string S;
    cin >> S;

    set<pair<int, int>> items;
    for (int i = 0; i < M; ++i) {
        int x, y;
        cin >> x >> y;
        items.insert({x, y});
    }

    int x = 0;
    int y = 0;

    for (char direction : S) {
        if (direction == 'R') {
            ++x;
        } else if (direction == 'L') {
            --x;
        } else if (direction == 'U') {
            ++y;
        } else {
            --y;
        }

        --H;
        if (H < 0) {
            cout << "No\n";
            return 0;
        }

        pair<int, int> position = {x, y};
        if (H < K && items.count(position)) {
            H = K;
            items.erase(position);
        }
    }

    cout << "Yes\n";
    return 0;
}
