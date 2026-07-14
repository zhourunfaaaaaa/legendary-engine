#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    vector<int> a(n);
    int sum = 0;

    for (int i = 0; i < n; i++) {
        cin >> a[i];
        sum += a[i];
    }

    int x1 = floor((double)sum / n);
    int x2 = ceil((double)sum / n);

    int ans1 = 0;
    int ans2 = 0;

    for (int i = 0; i < n; i++) {
        ans1 += (a[i] - x1) * (a[i] - x1);
        ans2 += (a[i] - x2) * (a[i] - x2);
    }

    cout << min(ans1, ans2) << endl;

    return 0;
}