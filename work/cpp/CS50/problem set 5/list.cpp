#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *list = (int*)malloc(sizeof(int)*3);
    if (list == NULL) {
        return 1;
    }
    list[0] = 1;
    list[1] = 2;
    list[2] = 3;

    int *tmp = (int*)realloc(list, 4*sizeof(int));
    if (tmp == NULL) {
        free(list);
        return 1;
    }

    tmp[3] = 4;

    for (int i = 0; i < 4; i++) {
        printf("%d\n", tmp[i]);
    }

    return 0;
}