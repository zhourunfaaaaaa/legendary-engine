#include <stdio.h>

int main()
{
    int groups = 6;
    int students = 8;
    int grades;
    int classgrades = 0;
    int i,j; 
    
    for (i = 1;i <= groups;i++) {
        int groupgrades = 0;
        printf("请输入第%d组的成绩:\n",i);
        for (j = 1;j <= students;j++) {
            scanf("%d",&grades);
            groupgrades += grades;
        }
        printf("第%d组的平均成绩是%f\n",i,groupgrades*1.0/students);
        classgrades += groupgrades;
    }
    printf("全班的平均成绩是%f",classgrades*1.0/students/groups);

    return 0;
}