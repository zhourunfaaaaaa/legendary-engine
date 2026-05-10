#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 4

typedef struct {
    char id[20];
    char name[50];
    float score[5];
    float ave;
} stu;

void task1()
{
    FILE *f1 = fopen("grade.txt", "w");
    if (f1 == NULL) return;
    char id[20], name[50];
    float score[5];
    for (int i = 0; i < N; i++) {
        printf("请输入第%d个学生的学号、姓名、5门课成绩：\n", i + 1);
        scanf("%s %s %f %f %f %f %f", id, name, 
              &score[0], &score[1], &score[2], &score[3], &score[4]);
        float avg = (score[0] + score[1] + score[2] + score[3] + score[4]) / 5.0;
        fprintf(f1, "%s;%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n", 
                id, name, score[0], score[1], score[2], score[3], score[4], avg);
    }
    fclose(f1);
    printf("\n保存成功！grade.txt 内容如下：\n");
    f1 = fopen("grade.txt", "r");
    char line[256];
    while (fgets(line, 256, f1)) printf("%s", line);
    fclose(f1);
}

void task2()
{
    FILE *f1 = fopen("grade.txt", "r");
    FILE *f2 = fopen("new_grade.txt", "w");
    if (f1 == NULL || f2 == NULL) return;
    char save[N][256];
    float ave[N];
    for (int i = 0; i < N; i++) {
        fgets(save[i], 256, f1);
        int cnt = 0;
        for (int j = 0; save[i][j] != '\0'; j++) {
            if (save[i][j] == ';') cnt++;
            if (cnt == 7) {
                ave[i] = atof(&save[i][j+1]);
                break;
            }
        }
    }
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1 - i; j++) {
            if (ave[j] < ave[j+1]) {
                float t1 = ave[j]; ave[j] = ave[j+1]; ave[j+1] = t1;
                char t2[256]; strcpy(t2, save[j]); 
                strcpy(save[j], save[j + 1]); strcpy(save[j + 1], t2);
            }
        }
    }
    for (int i = 0; i < N; i++) fputs(save[i], f2);
    fclose(f1); fclose(f2);
    printf("\n排序成功！new_grade.txt 内容如下：\n");
    f2 = fopen("new_grade.txt", "r");
    char line[256];
    while (fgets(line, 256, f2)) printf("%s", line);
    fclose(f2);
}

void task3()
{
    stu s[N];
    FILE *f1 = fopen("grade.dat", "wb");
    if (!f1) return;
    for(int i = 0; i < N; i++) {
        printf("请输入第%d个学生的学号、姓名、5门课成绩：\n", i + 1);
        scanf("%s %s %f %f %f %f %f", 
              s[i].id, s[i].name, 
              &s[i].score[0], &s[i].score[1], &s[i].score[2], 
              &s[i].score[3], &s[i].score[4]);
        s[i].ave = (s[i].score[0] + s[i].score[1] + s[i].score[2] + 
                    s[i].score[3] + s[i].score[4]) / 5.0;
        fwrite(&s[i], sizeof(stu), 1, f1);
    }
    fclose(f1);
}

void task4()
{
    stu s[N];
    FILE *f1 = fopen("grade.dat", "rb");
    if (f1 == NULL) return;
    for (int i = 0; i < N; i++) fread(&s[i], sizeof(stu), 1, f1);
    fclose(f1);
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1 - i; j++) {
            if (s[j].ave < s[j+1].ave) {
                stu t = s[j]; s[j] = s[j+1]; s[j+1] = t;
            }
        }
    }
    FILE *f2 = fopen("sorted_grade.dat", "wb");
    if (f2 == NULL) return;
    for (int i = 0; i < N; i++) fwrite(&s[i], sizeof(stu), 1, f2);
    fclose(f2);
}

void task5()
{
    char input;
    do {
        FILE *f1 = fopen("id.txt", "r");
        if (f1 == NULL) return;
        char id_pool[100][20];
        int count = 0;
        while (fscanf(f1, "%s", id_pool[count]) != EOF) count++;
        fclose(f1);
        if (count == 0) break;
        int n = rand() % count;
        char find_id[20];
        strcpy(find_id, id_pool[n]);
        printf("\n本次抽中的学号为：%s\n", find_id);
        f1 = fopen("id.txt", "w");
        for (int i = 0; i < count; i++) {
            if (i != n) fprintf(f1, "%s\n", id_pool[i]);
        }
        fclose(f1);
        FILE *f2 = fopen("grade.dat", "rb+");
        if (f2 == NULL) return;
        stu s; int found = 0; long pos = 0;
        while (fread(&s, sizeof(stu), 1, f2) == 1) {
            if (strcmp(s.id, find_id) == 0) {
                found = 1; s.score[0] += 10.0;
                s.ave = (s.score[0]+s.score[1]+s.score[2]+s.score[3]+s.score[4])/5.0;
                fseek(f2, pos, SEEK_SET);
                fwrite(&s, sizeof(stu), 1, f2);
                printf("加分成功！学号：%s, 新平均分：%.2f\n", find_id, s.ave);
                break;
            }
            pos = ftell(f2);
        }
        fclose(f2);
        if (found == 0) printf("查无此人！\n");
        printf("继续？(Y/n): ");
        scanf(" %c", &input);
    } while (input == 'Y' || input == 'y');
}

void task6()
{
    typedef struct {
        char id[20], name[50], origin[50], major[50];
        float score[5], ave;
    } stu_full;
    stu_full s[N] = {
        {"001", "Alice", "Beijing", "CS", {90, 90, 90, 90, 90}, 90.0},
        {"002", "Bob", "Shanghai", "CS", {80, 80, 80, 80, 80}, 80.0},
        {"003", "Charlie", "Wuhan", "Math", {70, 70, 70, 70, 70}, 70.0},
        {"004", "David", "Nanjing", "Math", {60, 40, 50, 20, 0}, 34.0}
    };
    char majors[N][50]; int m_cnt = 0;
    for (int i = 0; i < N; i++) {
        int found = 0;
        for (int j = 0; j < m_cnt; j++) if (strcmp(s[i].major, majors[j]) == 0) found = 1;
        if (!found) strcpy(majors[m_cnt++], s[i].major);
    }
    printf("\n分专业成绩统计\n");
    for (int m = 0; m < m_cnt; m++) {
        printf("专业: %s\n", majors[m]);
        for (int c = 0; c < 5; c++) {
            float temp[N], sum = 0, max, min; int num = 0;
            for (int i = 0; i < N; i++) {
                if (strcmp(s[i].major, majors[m]) == 0) {
                    temp[num] = s[i].score[c];
                    if (num == 0 || temp[num] > max) max = temp[num];
                    if (num == 0 || temp[num] < min) min = temp[num];
                    sum += temp[num]; num++;
                }
            }
            float mid = (num % 2 == 0) ? (temp[num/2-1] + temp[num/2])/2.0 : temp[num/2];
            printf("  课程%d: MAX=%.1f, MIN=%.1f, AVG=%.1f, MID=%.1f\n", c+1, max, min, sum/num, mid);
        }
    }
    printf("\n各门课分数段占比\n");
    for (int c = 0; c < 5; c++) {
        int d[5] = {0};
        for (int i = 0; i < N; i++) {
            float sc = s[i].score[c];
            if (sc == 0) d[0]++; else if (sc < 60) d[1]++;
            else if (sc < 75) d[2]++; else if (sc < 90) d[3]++; else d[4]++;
        }
        printf("  课程%d: 0级:%.1f%%, (0,60):%.1f%%, [60,75):%.1f%%, [75,90):%.1f%%, [90,100]:%.1f%%\n", 
               c+1, d[0]*100.0/N, d[1]*100.0/N, d[2]*100.0/N, d[3]*100.0/N, d[4]*100.0/N);
    }
}

int main()
{
    srand((unsigned int)time(NULL));
    task1(); task2(); task3(); task4(); task5(); task6();
    return 0;
}