#include <stdio.h>

int main()
{
    int hour,min;
    scanf("%d:%d",&hour,&min);
    if (hour / 12 == 0){
        printf("%d:%d AM",hour,min);
    } else if (hour == 12){
        printf("12:%d PM",min);
    } else if (hour == 24){
        printf("0:%d AM",min);
    } else{
        printf("%d:%d PM",hour-12,min);
    }

    return 0;
}