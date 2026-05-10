//水仙花数
#include <stdio.h>
#include <math.h>

int main()
{
    int n;
    scanf("%d",&n);
    int start = pow(10,n-1);
    int end = pow(10,n);
    for (int i = start; i<end; i++)
    {
        int num = i;
        int sum = 0;
        
        do {
            int digit = num%10;
            sum += pow(digit,3);
            num /= 10;
        } while (num > 0);
        
        if (sum == i) {
            printf("%d\n",i);
        }
    }
    return 0;
}

/*
错误记录：
1.边界错误，应该是10^n-1到10^n。
2.死循环，因为在内循坏中改变了外循环的循环变量，这里用int num = i来解决。
3.数字分解出现错误，n/10是去掉个位，n%10才是得到个位数字，n%=10是永远得到个位数字
n/=10才是去掉个位。
 */