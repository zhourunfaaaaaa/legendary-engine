#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    char s[] = "hello";
    char *p = strchr(s,'l');
    //(1) p = strchr(p,'l');

    //(2) char *t = (char*)malloc(strlen(p)+1);
    //    strcpy(t,p);
    //    printf("%s\n",t);
    //    free(t);

    char c = *p;
    *p = '\0';
    char *t = (char*)malloc(strlen(s)+1);
    strcpy(t,s);
    printf("%s\n",t);
    *p = c;
    free(t);

    return 0;
}