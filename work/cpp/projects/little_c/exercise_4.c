#include <stdio.h>

/*int main()
{
    int score;
    scanf("%d",&score);
    if (score >= 90){;
        printf("A");
    } else if (score >= 80){
        printf("B");
    } else if (score >= 70){
        printf("C");
    } else if (score >= 60){
        printf("D");
    } else {
        printf("E");
    }
    return 0;
}*/

int main()
{
    int score;
    scanf("%d",&score);
    score /= 10;
    switch (score){
        case 10:
        case 9:
        printf("A");
        break;
        case 8:
        printf("B");
        break;
        case 7:
        printf("C");
        break;
        case 6:
        printf("D");
        break;
        default:
        printf("E");
        break;
    }
}