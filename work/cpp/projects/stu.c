#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stu.h"

Student students[MAX_STUDENTS_NUMBER];
int student_count = 0;

Course courses[MAX_COURSES_NUMBER];
int course_count = 0;

Score scores[MAX_SCORES_NUMBER];
int score_count = 0;

Payment payments[MAX_PAYMENT_NUMBER];
int payment_count = 0;

Index students_index[MAX_STUDENTS_NUMBER];
int students_index_count = 0;

Index courses_index[MAX_COURSES_NUMBER];
int courses_index_count = 0;

int get_string(char* destination, int size)
{
    char buffer[MAX_INPUT_LENGTH];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    strncpy(destination, buffer, size - 1);
    destination[size - 1] = 0;
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
    char *end;
    long number = strtol(buffer, &end, 10);
    if (*end != '\n' && *end != '\0') {
        return 0;
    }
    *value = (int)number;
    return 1;
}

int get_float(float *value)
{
    char buffer[MAX_INPUT_LENGTH];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (buffer[0] == '\n') {
        return 0;
    }
    char *end;
    double number = strtod(buffer, &end);
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

int load_table(const char* file_name, void* buffer, int* count, int max, size_t record_size)
{
    FILE* fp = fopen(file_name, "rb");
    if (!fp) {
        *count = 0;
        return 0;
    }

    int record_conut;
    if (fread(&record_conut, sizeof(int), 1, fp) != 1) {
        *count = 0;
        return 0;
    }

    if (record_conut < 0 || record_conut > max) {
        *count = 0;
        return 0;
    }

    *count = fread(buffer, record_size, record_conut, fp);
    fclose(fp);
    return 1;
}

int save_table(const char* file_name, void* buffer, int* count, size_t record_size)
{
    FILE* fp = fopen(file_name, "wb");
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
