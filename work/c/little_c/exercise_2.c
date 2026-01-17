#include <stdio.h>

int main()
{
    int i;
    scanf("%d",&i);

    int foot = i / 30.48;
    int inch =  ((i / 30.48) - foot) * 12;

    printf("%d %d",foot,inch);

    return 0;
}