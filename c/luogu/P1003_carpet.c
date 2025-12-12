#include <stdio.h>
#include <stdlib.h>

int main()
{
    typedef struct {
        int a,b,x,y;
    } Carpet;
    int n;
    scanf("%d",&n);

    Carpet *carpet = (Carpet*)malloc(n * sizeof(Carpet));

    int i;
    for (i = 0;i < n;i++) {
        scanf("%d %d %d %d",&carpet[i].a,&carpet[i].b,&carpet[i].x,&carpet[i].y);
    }

    int check_x,check_y;
    scanf("%d %d",&check_x,&check_y);
    for (i = n-1;i >= 0;i--) {
        if (check_x <= carpet[i].x + carpet[i].a && check_x >= carpet[i].a &&
            check_y <= carpet[i].y + carpet[i].b && check_y >= carpet[i].b) {
                printf("%d",i+1);
                free(carpet);
                return 0;
            }
    }

    printf("-1");
    return 0;
}