#include <bits/stdc++.h>

using namespace std;

struct people {
    int time;
    int position;
};

bool cmp(people a, people b) {
    if (a.time == b.time) {
        return a.position < b.position;
    } else {
        return a.time < b.time;
    }
}
int main()
{
    int n;
    cin >> n;
    vector<people> t(n);

    for (int i = 0; i < n; i++) {
        t[i].position = i + 1;
        cin >> t[i].time;
    }

    sort(t.begin(), t.end(), cmp); // 用贪心 + sort 也可以实现，不用 map，因为不好处理位置关系
    double sum = 0;
    for (int i = 0; i < n; i++) {
        cout << t[i].position;
        sum += t[i].time * (n - i - 1);
        if (i != n - 1) {
            cout << " ";
        }
    }
    cout << endl;

    double ans = sum / n;

    printf("%.2lf\n", ans);

    return 0;
}
