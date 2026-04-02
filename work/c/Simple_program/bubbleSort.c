#include <stdio.h>

int main()
{
    int n;
    scanf("%d", &n);
    int s[n];
    int i, j;
    
    // 输入
    for (i = 0; i < n; i++) {
        scanf("%d", &s[i]);
    }
    
    // 正确的冒泡排序
    for (i = 0; i < n - 1; i++) {              // 外层循环：n-1 轮
        for (j = 0; j < n - 1 - i; j++) {      // 内层循环：比较到已排序部分前
            if (s[j] > s[j + 1]) {              // 比较相邻元素
                int t = s[j];
                s[j] = s[j + 1];
                s[j + 1] = t;
            }
        }
    }
    
    // 输出
    for (i = 0; i < n; i++) {
        printf("%d ", s[i]);
    }
    
    return 0;
}