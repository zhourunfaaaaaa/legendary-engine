 #include <bits/stdc++.h>

using namespace std;

int main()
{
	char s[13]; // 0 2 3 4 6 7 8 9 10
	int sum = 0,cnt = 1;
	for (int i = 0;i < 13;i++) {
		cin >> s[i];
		if (i != 12 && s[i] != '-') {
			sum += (s[i]-'0')*cnt; 
			cnt++;
		}
	} 
	int ans = sum%11;
    if (ans != 10) {
    	if (ans+'0' == s[12]) {
    		cout << "Right";
    	} else {
    		s[12] = ans + '0';
    		for (int i = 0;i < 13;i++) {
    			cout << s[i];
    		} 
    	}
    } else {
        if ('X' == s[12]) {
            cout << "Right";
        } else {
            s[12] = 'X';
            for (int i = 0;i < 13;i++) {
    			cout << s[i];
    		} 
        }
    }
	 
    // 注意一个char类型和int的转换
	return 0;
} 