#include <stdio.h>

int main()
{
    int A,b,c,d,e;
    scanf("%d",&A);
    for (b=A; b<A+4; b++)
    {
        for (c=A; c<A+4; c++)
        {
            for (d=A; d<A+4; d++)
            {
                if (b != c && b != d && c != d) {
                    printf("%d",b*100 + c*10 + d);
                    e++;
                    if (e%6 != 0) {
                        printf(" ");
                    } else {
                        printf("\n");
                    }
                }
            }
        }
    }
    return 0;
}