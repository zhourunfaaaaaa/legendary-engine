// 高效简洁的代码

#include <stdio.h>

// 输出点数的函数
void print_point(int p) {
    if (p >= 2 && p <= 9) {
        printf("%d", p);
    } else if (p == 10) {
        printf("10");
    } else if (p == 11) {
        printf("J");
    } else if (p == 12) {
        printf("Q");
    } else if (p == 13) {
        printf("K");
    } else if (p == 14) {
        printf("A");
    }
}

int main() {
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        int a[3];
        char s[3];
        scanf("%d%c %d%c %d%c", &a[0], &s[0], &a[1], &s[1], &a[2], &s[2]);

        // 排序点数（从大到小）
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                if (a[k] < a[k+1]) {
                    int t = a[k];
                    a[k] = a[k+1];
                    a[k+1] = t;
                    char tc = s[k];
                    s[k] = s[k+1];
                    s[k+1] = tc;
                }
            }
        }

        // 判断花色是否相同
        int same_suit = (s[0] == s[1] && s[1] == s[2]);
        // 判断是否豹子
        int leopard = (a[0] == a[1] && a[1] == a[2]);
        // 判断是否顺子
        int straight = (a[0]-1 == a[1] && a[1]-1 == a[2]);
        // 判断是否对子
        int pair = (a[0] == a[1] || a[1] == a[2]);

        // 按优先级输出
        if (leopard) {
            printf("Leopard ");
            print_point(a[0]);
            printf("\n");
        } else if (same_suit && straight) {
            printf("Flush straight ");
            print_point(a[0]);
            printf("\n");
        } else if (straight) {
            printf("Straight ");
            print_point(a[0]);
            printf("\n");
        } else if (same_suit) {
            printf("Same kind ");
            print_point(a[0]);
            printf("\n");
        } else if (pair) {
            // 对子：找出对子的点数
            int pair_point = (a[0] == a[1]) ? a[0] : a[1];
            printf("Pair ");
            print_point(pair_point);
            printf("\n");
        } else {
            printf("General ");
            print_point(a[0]);
            printf("\n");
        }
    }
    return 0;
}

// 以下是我写的屎山代码

/*
#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    for (int i = 0;i < n;i++) {
        int a[3];
        char b1,b2,b3;
        char c[3];
        
        scanf("%d%c %d%c %d%c",&a[0],&b1,&a[1],&b2,&a[2],&b3);
        for (int j = 0;j < 3;j++) {
            for (int k = 0;k < 2;k++) {
                if (a[k] < a[k+1]) {
                    int t = a[k];
                    a[k] = a[k+1];
                    a[k+1] = t;
                }
            }
        }
        if (a[0] != 10 || (a[1] == 10 && a[2] == 10)) {
            for (int j = 0;j < 3;j++) {
                if (a[j] >= 2 && a[j] <= 10) {
                    c[j] = a[j] +'0';
                }
                if (a[j] == 11) {
                    c[j] = 'J';
                }
                if (a[j] == 12) {
                    c[j] = 'Q';
                }
                if (a[j] == 13) {
                    c[j] = 'K';
                }
                if (a[j] == 14) {
                    c[j] = 'A';
                }
            }
            // 豹子
            if (c[0] == c[1] && c[1] == c[2]) {
                printf("Leopard %c\n",c[0]);
            }
            // 对子 
            else if ((c[0] == c[1]) + (c[1] == c[2]) + (c[0] == c[2]) == 1) {
                if (c[0] == c[1] || c[0] == c[2]) {
                    printf("Pair %c\n",c[0]);
                } else {
                    printf("Pair %c\n",c[1]);
                }
            }
            // 同花顺
            else if ((b1 == b2 && b1 == b3) && (a[0]-1 == a[1] && a[1]-1 == a[2])) {
                printf("Flush straight %c\n",c[0]);
            }
            // 顺子
            else if ((a[0]-1 == a[1] && a[1]-1 == a[2])) {
                printf("Straight %c\n",c[0]);
            }
            // 同花
            else if (b1 == b2 && b2 == b3) {
                printf("Same kind %c\n",c[0]);
            } 
            // 花牌
            else {
                printf("General %c\n",c[0]);
            }
        } else {
            // 豹子
            if (a[0] == a[1] && a[0] == a[2]) {
                printf("Leopard 10\n");
            }
            // 对子
            else if (a[1] == a[2]) {
                printf("Pair %d\n",a[1]);
            } else if (a[1] == a[0] && a[0] == a[2]) {
                printf("Pair 10\n");
            }
            // 同花
            else if (b1 == b2 && b1 == b3) {
                printf("Same kind %c",c[0]);
            }
            // 花牌
            else {
                printf("General 10\n");
            }
        }
    }
    return 0;
}
*/