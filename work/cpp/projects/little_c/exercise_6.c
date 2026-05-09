#include <stdio.h>

int main()
{
    int days;
    scanf("%d",&days);
    int i = days % 5;
    if (i == 0){
        printf("Drying in day %d",days);
    } else if (i > 3){
        printf("Drying in day %d",days);
    } else{
        printf("Fishing in day %d",days);
    }

    return 0;

}

/*
 
int main()
{
	int i;
	scanf("%d", &i);	
	if(i%5 <= 3 && i%5 != 0)
	     printf("Fishing in day %d", i);
	else 
	     printf("Drying in day %d", i);
	return 0;
} */