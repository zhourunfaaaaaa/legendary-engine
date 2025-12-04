#include <stdio.h>

int main()
{
    int n;
    scanf("%d",&n);
    int t = n;
    int digit = 0;
    int s[10];
    while (t > 0) {
        s[digit] = t%10;
        t /= 10;
        digit++;
    }

    int f = 1;
    int num[10] = {0};
    int i;
    for (i = 0;i < digit;i++) {
        num[s[i]]++;
        if (num[s[i]] > 1) {
            f = 0;
            break;
        
        }
    }
    if (f) {
        printf("YES!");
    } else {
        printf("NO!");
    }

    return 0;
}