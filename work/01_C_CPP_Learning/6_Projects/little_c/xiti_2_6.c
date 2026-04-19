#include <stdio.h>

int main()
{
    int n = 3;
    int sum = 0;
    for (;n < 1000;n++) {
        if (n%3 == 0) {
            sum += n;
        }
    }

    printf("%d",sum);

    return 0;
}

/*
int main()
{
    int n = 1;
    int sum = 0;
    while (n*3 <= 1000) {
        sum += n*3;
        n++;
    }

    printf("%d",sum);

    return 0;
}
*/