#include <stdio.h>

int main()
{
    int s[10];
    int i,j,maxIndex,t;
    
    for (i = 0;i < 10;i++) {
        scanf("%d",&s[i]);
    }

    for (i = 0;i < 10;i++) {
        maxIndex = i;
        for (j = 0;j <= 10;j++) {
            if (s[j] > s[maxIndex]) {
                maxIndex = j;
            }
        }
        if (maxIndex != i) {
            t = s[maxIndex];
            s[maxIndex] = s[i];
            s[i] = t;
        }
    }

    for (i = 0; i < 10; i++) {
        printf("%d ", s[i]);
    }

    return 0;
}
//下面是笨蛋方法
// #include <stdio.h>

// int main()
// {
//     int s[10];
//     int i,j;
//     int changeNum;
//     for (i = 0;i < 10;i++) {
//         scanf("%d",&s[i]);
//     }

//     for (i = 0;i < 10;i++) {
//         changeNum = s[i];
//         for (j = i;j < 10;j++) {
//             if (s[j] > s[i]) {
//                 changeNum = s[j];
//                 s[j] = s[i];
//                 s[i] = changeNum;
//             }
//         }
//     }

//     for (i = 0;i < 10;i++) {
//         printf("%d ",s[i]);
//     }

//     return 0;
// }
//以及冒泡算法
// #include <stdio.h>

// int main()
// {
//     int s[10];
//     int i, j, temp;
    
//     for (i = 0; i < 10; i++) {
//         scanf("%d", &s[i]);
//     }

//     // 冒泡排序（降序）
//     for (i = 0; i < 9; i++) {
//         for (j = 0; j < 9 - i; j++) {// 第一次看前十个元素，第二次看前九个元素
//             if (s[j] < s[j + 1]) {
//                 temp = s[j];
//                 s[j] = s[j + 1];
//                 s[j + 1] = temp;
//             }
//         }
//     }

//     for (i = 0; i < 10; i++) {
//         printf("%d ", s[i]);
//     }

//     return 0;
// }