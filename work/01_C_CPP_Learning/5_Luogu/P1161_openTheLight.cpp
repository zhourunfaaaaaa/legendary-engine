#include <stdio.h>
#include <math.h>

int main()
{
    int n;
    scanf("%d",&n);
    
    double a;
    int t;
    int i,j;
    int ans = 0;
    for (i = 0;i < n;i++) {
        scanf("%lf %d",&a,&t);
        for (j = 1;j <= t;j++) {
            ans = ans ^ (int)floor(j*a);
        }
    }

    printf("%d",ans);
    return 0;
}