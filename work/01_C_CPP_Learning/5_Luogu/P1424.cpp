#include <bits/stdc++.h>

using namespace std;

// 我的方法:数组扩容,不用二次取模
int main()
{
    int x,n;
    cin >> x >> n;
    int daysInWeek[] = {1,1,1,1,1,0,0,1,1,1,1,1,0,0};

    int weeks = n/7;
    int days = n%7;
    int s = 1250*weeks;
    for (int i = x;i < x+days;i++) {
        s += 250*daysInWeek[i-1];
    }
    cout << s;

    return 0;
}
// ai方法
// #include <iostream>
// using namespace std;

// int main() {
//     int w, n;
//     cin >> w >> n;
    
//     // 完整周的工作量
//     long long total = (long long)(n / 7) * 5 * 250;
    
//     // 处理剩余天数
//     int remain = n % 7;
//     for (int i = 0; i < remain; i++) {
//         int day = (w + i - 1) % 7 + 1;// 这样操作才能保证不会出现等于0的情况
//         if (day <= 5) {
//             total += 250;
//         }
//     }
    
//     cout << total << endl;
    
//     return 0;
// }