#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);
    
    int maxpos, minpos, maxneg, minneg;
    int f1 = 0, f2 = 0;
    
    for (int i = 0; i < n; i++) {
        int number;
        scanf("%d", &number);
        
        if (number > 0) {
            if (!f1) {
                maxpos = minpos = number;
                f1 = 1;
            } else {
                if (number > maxpos) maxpos = number;
                if (number < minpos) minpos = number;
            }
        } else if (number < 0) {
            if (!f2) {
                maxneg = minneg = number;
                f2 = 1;
            } else {
                if (number > maxneg) maxneg = number;
                if (number < minneg) minneg = number;
            }
        }
    }
    
    if (f1) {
        printf("最大正整数为%d\n", maxpos);
        printf("最小正整数为%d\n", minpos);
    } else {
        printf("不存在最大正整数\n");
        printf("不存在最小正整数\n");
    }
    
    if (f2) {
        printf("最大负整数为%d\n", maxneg);
        printf("最小负整数为%d\n", minneg);
    } else {
        printf("不存在最大负整数\n");
        printf("不存在最小负整数\n");
    }
    
    return 0;
}
/*
用了一个has来解决如何让当输入正数的时候，minpos = number只执行一次。
唯一没想到的就是这点！
*/