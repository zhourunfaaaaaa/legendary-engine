#include <bits/stdc++.h>
using namespace std;

struct Wood {
	int l;
	int w;
};

bool cmp(const Wood &a, const Wood &b) {
	if (a.l != b.l) {
		return a.l > b.l;
	}
	return a.w > b.w;
}

int main() {
	int n;
	cin >> n;
	vector<Wood> woods(n);
	vector<bool> vis(n);
	for (int i = 0; i < n; i++) {
		cin >> woods[i].l >> woods[i].w;
	}
	sort(woods.begin(),woods.end(),cmp); 
	
	int ans = 0;
	for (int i = 0; i < n; i++) {
		if (!vis[i]) {
			ans++;
			int cur = woods[i].w;
			vis[i] = true;
			for (int j = i+1; j < n; j++) {
				if (!vis[j] && woods[j].w <= cur) {
					cur = woods[j].w;
					vis[j] = true;
				}
			}
		}
	}
	
	cout << ans; 
	
    return 0;
}
