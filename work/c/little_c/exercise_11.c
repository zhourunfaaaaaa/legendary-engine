#include <stdio.h>

int run(int year);
int hui(int year);

int main() {
    int x, y;
    scanf("%d %d", &x, &y);
    
    //先输出闰年数量
    int runCount = 0;
    for (int i = x; i <= y; i++) {
        if (run(i)) {
            runCount++;
        }
    }
    printf("%d\n", runCount);
    
    //再依次输出闰年
    if (runCount > 0) {
        int first = 1;
        for (int i = x; i <= y; i++) {
            if (run(i)) {
                if (!first) {    //这个输出形式就可以保证最后没有空格
                    printf(" ");
                }
                printf("%d", i);
                first = 0;
            }
        }
        printf("\n");
    } else {
        printf("\n");
    }
    
    int huiCount = 0;
    for (int i = x; i <= y; i++) {
        if (hui(i)) {
            huiCount++;
        }
    }
    printf("%d\n", huiCount);
    
    if (huiCount > 0) {
        int first = 1;
        for (int i = x; i <= y; i++) {
            if (hui(i)) {
                if (!first) {
                    printf(" ");
                }
                printf("%d", i);
                first = 0;
            }
        }
        printf("\n");
    } else {
        printf("\n");
    }
    
    return 0;
}

int run(int year) {
    if (year % 100 == 0) {
        return year % 400 == 0;//是闰年返回1，不是返回0
    } else {
        return year % 4 == 0;
    }
}

int hui(int year) {
    int num = year;
    int reverse = 0;
    
    while (num > 0) {
        reverse = reverse * 10 + num % 10;
        num /= 10;
    }
    
    return reverse == year;//是回文年返回1
}