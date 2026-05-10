#ifndef _STU_
#define _STU_

#define MAX_ID_LENGTH 20
#define MAX_STUDENTS_NUMBER 1000
#define MAX_COURSES_NUMBER 100
#define MAX_SCORES_NUMBER 5000
#define MAX_NAME_LENGTH 51
#define MAX_PAYMENT_NUMBER 2000
#define MAX_INPUT_LENGTH 256

typedef struct {
    char ID[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char birth[11];
    char major[MAX_NAME_LENGTH];
    char class[11];
    char gender;
} Student;

typedef struct {
    char code[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    int credits;
    int hours;
} Course;

typedef struct {
    char ID[MAX_ID_LENGTH];
    char code[MAX_COURSES_NUMBER];
    float scores;
    int semester;
} Score;

typedef struct {
    char number[MAX_ID_LENGTH];
    char ID[MAX_ID_LENGTH];
    char date[11];
    float amount;
} Payment;

typedef struct {
    char key[MAX_ID_LENGTH];
    int position;
} Index;

extern Student students[MAX_STUDENTS_NUMBER];
extern int student_count;

extern Course courses[MAX_COURSES_NUMBER];
extern int course_count;

extern Score scores[MAX_SCORES_NUMBER];
extern int score_count;

extern Payment payments[MAX_PAYMENT_NUMBER];
extern int payment_count;

extern Index students_index[MAX_STUDENTS_NUMBER];
extern int students_index_count;

extern Index courses_index[MAX_COURSES_NUMBER];
extern int courses_index_count;

int get_string(char* destination, int size);

int get_int(int *value);

int get_float(float *value);

int validate_date(const char* date);

int load_table(const char* file_name, void* buffer, int* count, int maximum, size_t record_size);

int save_table(const char* file_name, void* buffer, int* count, size_t record_size);

#endif
