#include <stdio.h>

int main()
{
    int l,m;
    int sum = 0;
    scanf("%d %d",&l,&m);
    int tree[l];
    int i;
    for (i = 0;i < l;i++) {
        tree[i] = 1;
    }

    int u,v;
    for (i = 0;i < m;i++) {
        scanf("%d %d",&u,&v);
        for (int j = u-1;j < v;j++) {
            tree[j] = 0;
        }
    }

    for (i = 0;i < l;i++) {
        if (tree[i] == 1) {
            sum++;
        }
    }

    printf("%d",sum);
    
    return 0;
}

//下为更好的方法

/*
#include <stdio.h>

int main() {
    int l, m;
    scanf("%d %d", &l, &m);
    
    int diff[l + 2]; // 需要 l+2，因为可能有 v+1 操作
    
    // 初始化差分数组为0
    for (int i = 0; i <= l + 1; i++) {
        diff[i] = 0;
    }
    
    // 处理每个区间
    for (int i = 0; i < m; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        diff[u] += 1;      // 区间开始
        diff[v + 1] -= 1;  // 区间结束的下一个位置
    }
    
    // 通过前缀和恢复原数组，并统计剩余树木
    int sum = 0, current = 0;
    for (int i = 0; i <= l; i++) {
        current += diff[i];  // current 表示当前位置被多少个区间覆盖
        if (current == 0) {  // 没有被任何区间覆盖，树还在
            sum++;
        }
    }
    
    printf("%d", sum);
    return 0;
}
*/