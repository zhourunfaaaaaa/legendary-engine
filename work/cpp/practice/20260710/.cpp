#include <bits/stdc++.h>
using namespace std;

int main()
{
    string n;
    cin >> n;
    int l = n.length();
    int cut[] = {7, 6};
    int i = 0;
    int cnt = 0;
    long long ans = 0;

    while (i < l) {
        long long sum = 0;
        // 可以去掉if-else以及l - i < 7的硬编码，用这一招：int len = min(cut[cnt & 1], (int)n.length() - i);
        if (l - i < 7) {
            for ( ; i < l; i++) {
                sum = sum * 10 + (n[i] - '0');
            }
        } else {
            for (int j = 0; j < cut[cnt & 1]; j++) {
                sum = sum * 10 + (n[i] - '0');
                i++;
            }
            cnt++;
        }
        ans += sum;
    }

    cout << ans << endl;

    return 0;
}