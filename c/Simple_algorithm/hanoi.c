#include <stdio.h>

int step = 1;
void hanoi(int n,char a,char b,char c) {
    if (n > 0) {
        hanoi(n-1,a,c,b);
        printf("Step %d :%d form %c to %c\n",step++,n,a,c);
        hanoi(n-1,b,a,c);
    }
}

int main()
{
    int n;
    scanf("%d",&n);
    hanoi(n,'A','B','C');
    return 0;
}
