#include <stdio.h>

enum color {red,green,yellow};

void f(enum color t);

int main()
{
    enum color t = red;

    scanf("%d",&t);
    f(t);

    return 0;
}

void f(enum color t) {
    printf("%d\n",t);
}