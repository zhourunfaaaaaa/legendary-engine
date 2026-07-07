#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "stu.h"

Student students[MAX_STUDENTS_NUMBER];// 这里才是具体的定义
int student_count = 0;

Course courses[MAX_COURSES_NUMBER];
int course_count = 0;

Score scores[MAX_SCORES_NUMBER];
int score_count = 0;

Payment payments[MAX_PAYMENT_NUMBER];
int payment_count = 0;

Index students_index[MAX_STUDENTS_NUMBER];
int student_index_count = 0;

Index courses_index[MAX_COURSES_NUMBER];
int course_index_count = 0;

int get_string(char* destination, int size)
{
    char buffer[MAX_INPUT_LENGTH];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = 0;// strcspn(a, b)就是检测字符串a里是否有字符在b中存在，返回第一个符合字符的位置，然后再把buffer的这个位置改为\0
    if (buffer[0] == '\0') {
        return 0;
    }
    strncpy(destination, buffer, size - 1);// strcpy是拷贝到\0才停止，而strncpy有长度限制
    destination[size - 1] = 0;// 再给destination末尾加上\0
    return 1;
}

int input_required(const char* prompt, char* destination, int size)
{
    printf("%s：", prompt);
    if (!get_string(destination, size)) {
        printf("错误：%s不能为空！\n", prompt);
        return 0;
    }
    return 1;
}

int get_int(int* value)
{
    char buffer[MAX_INPUT_LENGTH];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (buffer[0] == '\n') {
        return 0;
    }
    char* end;
    long number = strtol(buffer, &end, 10);// strtol返回long类型的值
    if (*end != '\n' && *end != '\0') {
        return 0;
    }
    *value = (int)number;
    return 1;
}

int get_float(float* value)
{
    char buffer[MAX_INPUT_LENGTH];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (buffer[0] == '\n') {
        return 0;
    }
    char* end;
    double number = strtod(buffer, &end);// strtod返回double类型的值
    if (*end != '\n' && *end != '\0') {
        return 0;
    }
    *value = (float)number;
    return 1;
}

int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int validate_date(const char* date)
{
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') return 0;

    int year = atoi(date), month = atoi(date + 5), day = atoi(date + 8);

    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) return 0;

    static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_day = days_in_month[month];
    if (month == 2 && is_leap_year(year)) max_day = 29;

    return day <= max_day;
}

int load_table(const char* file_name, void* buffer, int* count, int max, size_t record_size)// buffer接收任何类型的结构体数组，record_size每条记录的大小
{
    FILE* fp = fopen(file_name, "rb");// rb表示二进制只读
    if (!fp) {
        *count = 0;// 错误读取表示文件可能损坏，还需要把count清零
        return 0;
    }

    int record_conut;// 读取文件头部的条数，这是原始数据中的，如果损坏无法读取，清零返回失败
    if (fread(&record_conut, sizeof(int), 1, fp) != 1) {// 这里函数指针已经移动到第一个数据的位置了，所以后面第二次用fread读取的时候，指针直接从这里开始移动，不会读入计数
        *count = 0;
        return 0;
    }

    if (record_conut < 0 || record_conut > max) {// 条数数据有误，清零返回失败
        *count = 0;
        return 0;
    }

    *count = fread(buffer, record_size, record_conut, fp);// 读取数据，存入buffer里，并且更新实际读取到的数据数量，修改count
    fclose(fp);
    return 1;
}

int save_table(const char* file_name, void* buffer, int count, size_t record_size)// 这里count的类型不是指针是因为不需要修改count的值
{
    FILE* fp = fopen(file_name, "wb");// wb表示二进制写入
    if (!fp) {
        return 0;
    }

    fwrite(&count, sizeof(int), 1, fp);
    if (count > 0) {
        fwrite(buffer, record_size, count, fp);
    }
    fclose(fp);
    return 1;
}

void load_all(void)
{
    load_table("student.dat", students, &student_count, MAX_STUDENTS_NUMBER, sizeof(Student));
    load_table("course.dat", courses, &course_count, MAX_COURSES_NUMBER, sizeof(Course));
    load_table("score.dat", scores, &score_count, MAX_SCORES_NUMBER, sizeof(Score));
    load_table("payment.dat", payments, &payment_count, MAX_PAYMENT_NUMBER, sizeof(Payment));
}

void save_all(void)
{
    save_table("student.dat", students, student_count, sizeof(Student));
    save_table("course.dat", courses, course_count, sizeof(Course));
    save_table("score.dat", scores, score_count, sizeof(Score));
    save_table("payment.dat", payments, payment_count, sizeof(Payment));
    printf("数据已保存。\n");
}

int index_compare(const void* a, const void* b)// 类似c++的cmp，这里需要参数，但传给qsort的是函数指针，qsort自己会调用函数
{
    return strcmp(((Index*)a)->key, ((Index*)b)->key);
}

void build_student_index(void)
{
    student_index_count = student_count;
    for (int i = 0; i < student_index_count; i++) {
        strcpy(students_index[i].key, students[i].ID);
        students_index[i].position = i;// position对应在students里的下标，通过index排序之后，就可以实现通过position直接访问
    }
    qsort(students_index, student_index_count, sizeof(Index), index_compare);// 类似c++的sort，但是qsort的比较函数必须返回int
}

void build_course_index(void)
{
    course_index_count = course_count;
    for (int i = 0; i < course_index_count; i++) {
        strcpy(courses_index[i].key, courses[i].code);
        courses_index[i].position = i;
    }
    qsort(courses_index, course_index_count, sizeof(Index), index_compare);
}

int binary_research_index(Index* index, int count, const char* key)
{
    int left = 0, right = count - 1;
    while (left <= right) {
        int middle = (left + right) / 2;
        int compare_result = strcmp(key, index[middle].key);
        if (compare_result == 0) {
            return index[middle].position;
        } else if (compare_result < 0) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }
    return -1;
}

int find_student(const char* id)
{
    return binary_research_index(students_index, student_index_count, id);
}

int find_course(const char* code)
{
    return binary_research_index(courses_index, course_index_count, code);
}

void build_index(void)
{
    build_student_index();
    build_course_index();
}

void add_student(void)
{
    if (student_count >= MAX_STUDENTS_NUMBER) {
        printf("错误：学生数量已达上限！");
        return;
    }

    Student s;
    char buffer[MAX_INPUT_LENGTH];

    if (!input_required("学号", s.ID, sizeof(s.ID))) {
        return;
    }
    if (find_student(s.ID) >= 0) {
        printf("错误：学号已存在！\n");
        return;
    }

    if (!input_required("姓名", s.name, sizeof(s.name))) {
        return;
    }
    
    do {
        printf("性别 (M/F)：");
        get_string(buffer, sizeof(buffer));
        s.gender = (char)toupper(buffer);
    } while (s.gender != 'M' && s.gender != 'F');// 这里的s只是临时变量，就算存入错误数据，不会影响原来的数据

    do {
        printf("出生日期 (YYYY-MM-DD)：");
        get_string(buffer, sizeof(buffer));
    } while (validate_date(buffer));
    strcpy(s.birth, buffer);

    printf("专业：");
    getstr(s.major, sizeof(s.major));
    printf("班级：");
    getstr(s.class, sizeof(s.class));

    students[student_count++] = s;
    build_student_index();
    printf("学生添加成功！");
}