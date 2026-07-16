#include <bits/stdc++.h>
using namespace std;

int main() {
    int W, H, N;
    cin >> W >> H >> N;

    int left = 0, right = W;
    int bottom = 0, top = H;

    while (N--) {
        int x, y, a;
        cin >> x >> y >> a;

        if (a == 1) left = max(left, x);
        if (a == 2) right = min(right, x);
        if (a == 3) bottom = max(bottom, y);
        if (a == 4) top = min(top, y);
    }

    int width = max(0, right - left);
    int height = max(0, top - bottom);

    cout << width * height << '\n';
    return 0;
}