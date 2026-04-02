#include <bits/stdc++.h>

using namespace std;

int main()
{
    int n,s;
    cin >> n >> s;
    vector<int> p(n);
    vector<int> c(n);
    int ans = 1e9;
    for (int i = 0; i < n; i++) {
		cin >> p[i] >> c[i];
	}
	int max_times = *max_element(c.begin(),c.end());
	for (int time1 = 0; time1 <= max_times; time1++) {
		int cost1 = time1 * s;
		int cost2 = 0;
		for (int i = 0; i < n; i++) {
			int time2 = max(c[i]-time1,0);
			cost2 += p[i] * time2;
		}
		ans = min(ans, cost1+cost2);
	}
	cout << ans;
	 
    return 0;
}