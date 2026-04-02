#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_STUDENTS 30

typedef struct {
    int id; 
    int math; 
    int english; 
    int c_program;
    int total;       
    int rank;
} Student;

void generate_student_data(Student students[]);
void write_to_score_txt(Student students[]);
void read_from_score_txt(Student students[]);
void calculate_totals(Student students[]);
void sort_by_total_desc(Student students[]);
void print_students_by_total(Student students[]);
void sort_by_id(Student students[]);
void calculate_ranks(Student students[]);
void write_to_newscore_txt(Student students[]);

int main() {
    Student students[NUM_STUDENTS];
    
    
    generate_student_data(students);
    write_to_score_txt(students);
    
    read_from_score_txt(students);
    
    calculate_totals(students);
    
    sort_by_total_desc(students);
    print_students_by_total(students);
    printf("\n");
    
    calculate_ranks(students);
    write_to_newscore_txt(students);
    
    return 0;
}

void generate_student_data(Student students[]) {
    srand(12345);
    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        students[i].id = 2024001 + i;
        students[i].math = 60 + rand() % 41;
        students[i].english = 60 + rand() % 41;
        students[i].c_program = 60 + rand() % 41; 
        students[i].total = 0;
        students[i].rank = 0;
    }
}

void write_to_score_txt(Student students[]) {
    FILE *fp = fopen("score.txt", "w");
    if (fp == NULL) {
        printf("错误：无法创建score.txt文件\n");
        exit(1);
    }
    
    fprintf(fp, "学号 数学 英语  C语言\n");
    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        fprintf(fp, "%d %d %d %d\n", 
                students[i].id, 
                students[i].math, 
                students[i].english, 
                students[i].c_program);
    }
    
    fclose(fp);
}

void read_from_score_txt(Student students[]) {
    FILE *fp = fopen("score.txt", "r");
    if (fp == NULL) {
        printf("错误：无法打开score.txt文件\n");
        exit(1);
    }
    
    char header[100];
    fgets(header, sizeof(header), fp);
    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        fscanf(fp, "%d %d %d %d", 
               &students[i].id, 
               &students[i].math, 
               &students[i].english, 
               &students[i].c_program);
        students[i].total = 0;
        students[i].rank = 0;
    }
    
    fclose(fp);
}

void calculate_totals(Student students[]) {
    for (int i = 0; i < NUM_STUDENTS; i++) {
        students[i].total = students[i].math + students[i].english + students[i].c_program;
    }
}

void sort_by_total_desc(Student students[]) {
    for (int i = 0; i < NUM_STUDENTS - 1; i++) {
        for (int j = 0; j < NUM_STUDENTS - i - 1; j++) {
            if (students[j].total < students[j + 1].total) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}

void print_students_by_total(Student students[]) {
    printf("=============================================\n");
    printf("排名 学号    数学  英语  C语言  总分\n");
    printf("=============================================\n");
    
    int current_rank = 1;
    for (int i = 0; i < NUM_STUDENTS; i++) {
        if (i > 0 && students[i].total < students[i - 1].total) {
            current_rank = i + 1;
        }
        
        printf("%-4d %-7d %-4d  %-4d  %-5d  %-4d\n",
               current_rank,
               students[i].id,
               students[i].math,
               students[i].english,
               students[i].c_program,
               students[i].total);
    }
    printf("=============================================\n");
}

void sort_by_id(Student students[]) {
    for (int i = 0; i < NUM_STUDENTS - 1; i++) {
        for (int j = 0; j < NUM_STUDENTS - i - 1; j++) {
            if (students[j].id > students[j + 1].id) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}

void calculate_ranks(Student students[]) {
    int current_rank = 1;
    int same_score_count = 1;
    
    sort_by_total_desc(students);
    
    students[0].rank = 1;
    
    for (int i = 1; i < NUM_STUDENTS; i++) {
        if (students[i].total == students[i - 1].total) {
            students[i].rank = current_rank;
            same_score_count++;
        } else {
            current_rank += same_score_count;
            students[i].rank = current_rank;
            same_score_count = 1;
        }
    }
    
    sort_by_id(students);
}

void write_to_newscore_txt(Student students[]) {
    FILE *fp = fopen("newscore.txt", "w");
    if (fp == NULL) {
        printf("错误：无法创建newscore.txt文件\n");
        exit(1);
    }
    
    fprintf(fp, "学号 数学 英语 C语言 总分 排名\n");
    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        fprintf(fp, "%d %d %d %d %d %d\n", 
                students[i].id, 
                students[i].math, 
                students[i].english, 
                students[i].c_program,
                students[i].total,
                students[i].rank);
    }
    
    fclose(fp);
}