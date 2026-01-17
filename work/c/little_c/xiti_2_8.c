#include <stdio.h>

int main()
{
    int n;
    int pos = 0,neg = 0,zero = 0;
    int pos_sum = 0,neg_sum = 0;
    for (int i = 0;i < 20;i++) {
        scanf("%d",&n);
        if (n > 0) {
            pos++;
            pos_sum += n;
        } else if (n < 0) {
            neg++;
            neg_sum += n;
        } else {
            zero++;
        }
    }

    printf("正整数%d个 负整数%d个 “0”%d个\n",pos,neg,zero);
    printf("正整数和为%d，平均数为%f\n",pos_sum,1.0*pos_sum/pos);
    printf("负整数和为%d，平均数为%f\n",pos_sum,1.0*pos_sum/pos);

    return 0;
}