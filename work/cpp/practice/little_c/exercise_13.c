#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);
    
    int a[200005], b[200005];
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    
    int k = 0;
    for (int i = 0; i < n; ) {// 这一段代码用来去重，第三个条件为空是因为下面while必然会执行
        b[k] = a[i];
        k++;
        while (i < n && a[i] == b[k-1]) {// 
            i++;// 如果重复，跳过a中的一个元素
        }
    }
    
    if (k <= 2) {
        printf("0\n");
    } else {
        int ans = 0;
        for (int i = 1; i < k - 1; i++) {
            if (b[i] > b[i - 1] && b[i] > b[i + 1]) {
                ans++;
            }
        }
        printf("%d\n", ans);
    }
    
    return 0;
}