#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>    // 标准输入输出库
#include <stdlib.h>   // 标准库函数（内存分配、程序控制等）
#include <string.h>   // 字符串操作函数
#include <ctype.h>    // 字符类型判断函数

/* ===== 常量定义 ===== */
#define MAX_STU  1000  // 最大学生数量（1000人）
#define MAX_COU  100   // 最大课程数量（100门）
#define MAX_SCO  5000  // 最大成绩数量（5000条）
#define MAX_PAY  2000  // 最大缴费数量（2000条）
#define MAX_ID   20    // ID字段最大长度（20字符）
#define MAX_NM   51    // 名称字段最大长度（51字符）

/* ===== 数据结构定义 ===== */

// 学生信息结构体
typedef struct {
    char id[MAX_ID];      // 学号（如：S20210001）
    char name[MAX_NM];    // 姓名（如：张三）
    char birth[11];       // 出生日期（格式：YYYY-MM-DD，11字符包括'\0'）
    char major[MAX_NM];   // 专业（如：计算机科学与技术）
    char cls[11];         // 班级（如：2021级1班）
    char gender;          // 性别（'M'表示男，'F'表示女）
} Student;

// 课程信息结构体
typedef struct {
    char code[MAX_ID];    // 课程代码（如：CS101）
    char name[MAX_NM];    // 课程名称（如：程序设计基础）
    int credit;           // 学分（整数，1-10之间）
    int hours;            // 学时（整数，总课时数）
} Course;

// 成绩信息结构体
typedef struct {
    char sid[MAX_ID];     // 学生学号（外键，关联Student.id）
    char cid[MAX_ID];     // 课程代码（外键，关联Course.code）
    float score;          // 成绩（浮点数，0-100之间）
    int semester;         // 学期（整数，如：20231表示2023年春季学期）
} Score;

// 缴费信息结构体
typedef struct {
    char pid[MAX_ID];     // 缴费编号（唯一标识每笔缴费）
    char sid[MAX_ID];     // 学生学号（外键，关联Student.id）
    char date[11];        // 缴费日期（格式：YYYY-MM-DD）
    float amount;         // 缴费金额（浮点数）
} Payment;

/* ===== 全局数据变量 ===== */
// 存储所有数据的数组和对应的计数器
static Student students[MAX_STU]; static int stu_cnt;  // 学生数组和计数器
static Course  courses[MAX_COU];  static int cou_cnt;  // 课程数组和计数器
static Score   scores[MAX_SCO];   static int sco_cnt;  // 成绩数组和计数器
static Payment payments[MAX_PAY]; static int pay_cnt;  // 缴费数组和计数器

/* ===== 索引结构（用于快速查找）===== */
// 索引节点结构体：将ID映射到数组位置，实现快速查找
typedef struct {
    char key[MAX_ID];  // 索引键（学号或课程代码）
    int pos;          // 对应的数组位置
} Idx;

// 为学生和课程建立索引，提高查找效率
static Idx stu_idx[MAX_STU]; static int stu_ic;  // 学生ID索引数组和计数器
static Idx cou_idx[MAX_COU]; static int cou_ic;  // 课程代码索引数组和计数器

/* ===== 输入工具函数 ===== */
// 通用字符串输入函数：安全地从标准输入读取字符串
static int getstr(char* dst, int sz) {
    char buf[256];  // 临时缓冲区，防止输入过长
    if (!fgets(buf, sizeof(buf), stdin)) return 0;  // 读取一行到缓冲区
    buf[strcspn(buf, "\n")] = 0;  // 找到换行符位置并替换为字符串结束符
    strncpy(dst, buf, sz - 1);    // 复制到目标字符串，防止溢出
    dst[sz - 1] = 0;              // 确保字符串以\0结尾
    return 1;                     // 返回成功标志
}

// 通用整数输入函数：安全地从标准输入读取整数
static int getint(int* v) {
    char buf[32];  // 临时缓冲区
    if (!fgets(buf, sizeof(buf), stdin)) return 0;  // 读取输入
    if (buf[0] == '\n') return 0;  // 如果只是回车，返回失败

    char* e;  // 错误指针
    long n = strtol(buf, &e, 10);  // 将字符串转换为长整型
    if (*e != '\n' && *e != 0) return 0;  // 检查是否完全转换
    *v = (int)n;  // 转换为int并保存
    return 1;     // 返回成功
}

// 通用浮点数输入函数：安全地从标准输入读取浮点数
static int getfloat(float* v) {
    char buf[32];  // 临时缓冲区
    if (!fgets(buf, sizeof(buf), stdin)) return 0;  // 读取输入

    char* e;  // 错误指针
    double d = strtod(buf, &e);  // 将字符串转换为双精度浮点数
    if (*e != '\n' && *e != 0) return 0;  // 检查转换是否完整
    *v = (float)d;  // 转换为float并保存
    return 1;       // 返回成功
}

/* ===== 数据验证函数 ===== */
// 判断是否为闰年
static int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// 验证日期格式的有效性
static int val_date(const char* d) {
    // 检查基本格式：必须是YYYY-MM-DD格式，共10个字符
    if (strlen(d) != 10 || d[4] != '-' || d[7] != '-') return 0;

    // 提取年、月、日
    int y = atoi(d), m = atoi(d + 5), day = atoi(d + 8);

    // 验证年份范围和月份范围
    if (y < 1900 || y > 2100 || m < 1 || m > 12 || day < 1) return 0;

    // 每月天数（下标0占位）
    static const int mdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_day = mdays[m];
    if (m == 2 && is_leap(y)) max_day = 29;

    return day <= max_day;
}

/* ===== 文件读写操作 ===== */
// 通用表格加载函数：从文件加载数据到内存
static int load_tbl(const char* fn, void* buf, int* cnt, int max, size_t sz) {
    FILE* fp = fopen(fn, "rb");  // 以二进制读模式打开文件
    if (!fp) { *cnt = 0; return 1; }  // 文件不存在时清零计数器并返回成功

    int n;  // 文件中声明的记录数量
    if (fread(&n, sizeof(int), 1, fp) != 1) {  // 读取记录数量
        fclose(fp);
        *cnt = 0;
        return 0;  // 读取失败
    }

    // 防止恶意文件：检查记录数量是否在合理范围内
    if (n < 0 || n > max) n = 0;

    // 读取实际数据到缓冲区
    *cnt = (int)fread(buf, sz, n, fp);  // 返回实际读取的记录数
    fclose(fp);  // 关闭文件
    return 1;    // 返回成功
}

// 通用表格保存函数：将内存数据保存到文件
static int save_tbl(const char* fn, const void* buf, int cnt, size_t sz) {
    FILE* fp = fopen(fn, "wb");  // 以二进制写模式打开文件
    if (!fp) return 0;  // 打开失败

    fwrite(&cnt, sizeof(int), 1, fp);  // 先写入记录数量
    if (cnt > 0) fwrite(buf, sz, cnt, fp);  // 再写入实际数据
    fclose(fp);  // 关闭文件
    return 1;    // 返回成功
}

// 加载所有数据文件
static void load_all(void) {
    // 按顺序加载各个数据文件
    load_tbl("stu.dat", students, &stu_cnt, MAX_STU, sizeof(Student));  // 加载学生数据
    load_tbl("cou.dat", courses, &cou_cnt, MAX_COU, sizeof(Course));   // 加载课程数据
    load_tbl("sco.dat", scores, &sco_cnt, MAX_SCO, sizeof(Score));    // 加载成绩数据
    load_tbl("pay.dat", payments, &pay_cnt, MAX_PAY, sizeof(Payment));  // 加载缴费数据
}

// 保存所有数据文件
static void save_all(void) {
    // 按顺序保存各个数据文件
    save_tbl("stu.dat", students, stu_cnt, sizeof(Student));  // 保存学生数据
    save_tbl("cou.dat", courses, cou_cnt, sizeof(Course));   // 保存课程数据
    save_tbl("sco.dat", scores, sco_cnt, sizeof(Score));    // 保存成绩数据
    save_tbl("pay.dat", payments, pay_cnt, sizeof(Payment));  // 保存缴费数据
    printf("数据已保存。\n");  // 提示用户保存完成
}

/* ===== 索引操作函数 ===== */
// 索引比较函数：用于qsort排序
static int idx_cmp(const void* a, const void* b) {
    // 比较两个索引节点的key值
    return strcmp(((Idx*)a)->key, ((Idx*)b)->key);
}

// 构建学生ID索引：将学生ID映射到数组位置，便于快速查找
static void build_stu_idx(void) {
    stu_ic = stu_cnt;  // 设置索引计数器
    for (int i = 0; i < stu_cnt; i++) {
        strcpy(stu_idx[i].key, students[i].id);  // 复制学号作为索引键
        stu_idx[i].pos = i;                      // 记录对应的数组位置
    }
    qsort(stu_idx, stu_ic, sizeof(Idx), idx_cmp);  // 按学号排序索引
}

// 构建课程代码索引：将课程代码映射到数组位置
static void build_cou_idx(void) {
    cou_ic = cou_cnt;  // 设置索引计数器
    for (int i = 0; i < cou_cnt; i++) {
        strcpy(cou_idx[i].key, courses[i].code);  // 复制课程代码作为索引键
        cou_idx[i].pos = i;                       // 记录对应的数组位置
    }
    qsort(cou_idx, cou_ic, sizeof(Idx), idx_cmp);  // 按课程代码排序索引
}

// 二分查找索引：在已排序的索引中查找指定key，返回对应的位置
static int bsearch_idx(Idx* idx, int cnt, const char* key) {
    int l = 0, r = cnt - 1;  // 左右边界
    while (l <= r) {         // 二分查找循环
        int m = (l + r) / 2; // 中间位置
        int c = strcmp(idx[m].key, key);  // 比较中间元素的key
        if (c == 0) return idx[m].pos;    // 找到匹配项，返回位置
        if (c < 0) l = m + 1; else r = m - 1;  // 调整搜索范围
    }
    return -1;  // 未找到，返回-1
}

// 便捷函数：根据学号查找学生位置
static int find_stu(const char* id) {
    return bsearch_idx(stu_idx, stu_ic, id);  // 在学生索引中查找
}

// 便捷函数：根据课程代码查找课程位置
static int find_cou(const char* code) {
    return bsearch_idx(cou_idx, cou_ic, code);  // 在课程索引中查找
}

// 更新所有索引：当数据发生变化时调用
static void build_idx(void) {
    build_stu_idx();  // 重建学生索引
    build_cou_idx();  // 重建课程索引
}

/* ===== 学生管理功能 ===== */
// 添加学生功能
static void add_stu(void) {
    if (stu_cnt >= MAX_STU) {
        printf("错误：学生数量已达上限！\n");
        return;
    }

    Student s;  // 创建临时学生结构体
    char buf[100];  // 临时缓冲区

    // 输入学号并验证
    printf("学号：");
    getstr(s.id, sizeof(s.id));  // 获取学号
    if (!s.id[0]) {
        printf("错误：学号不能为空！\n");
        return;
    }
    if (find_stu(s.id) >= 0) {
        printf("错误：学号已存在！\n");
        return;
    }

    // 输入姓名并验证
    printf("姓名：");
    getstr(s.name, sizeof(s.name));  // 获取姓名
    if (!s.name[0]) {
        printf("错误：姓名不能为空！\n");
        return;
    }

    // 输入性别（循环直到输入有效）
    do {
        printf("性别 (M/F)：");
        getstr(buf, sizeof(buf));
        s.gender = (char)toupper(buf[0]);  // 转换为大写
    } while (s.gender != 'M' && s.gender != 'F');  // 只接受M或F

    // 输入出生日期（循环直到输入有效）
    do {
        printf("出生日期 (YYYY-MM-DD)：");
        getstr(buf, sizeof(buf));
    } while (!val_date(buf));  // 验证日期格式
    strcpy(s.birth, buf);    // 保存有效日期

    // 输入专业和班级
    printf("专业：");
    getstr(s.major, sizeof(s.major));
    printf("班级：");
    getstr(s.cls, sizeof(s.cls));

    // 将学生信息添加到数组
    students[stu_cnt++] = s;
    build_stu_idx();  // 更新索引
    printf("学生添加成功！\n");
}

// 修改学生信息功能
static void mod_stu(void) {
    char id[MAX_ID];  // 学号缓冲区
    printf("请输入要修改的学生学号：");
    getstr(id, sizeof(id));  // 输入学号

    int pos = find_stu(id);  // 查找学生位置
    if (pos < 0) {
        printf("未找到该学生！\n");
        return;
    }

    Student* s = &students[pos];  // 获取学生指针
    // 显示当前信息
    printf("当前信息：%s | %s | %c | %s | %s | %s\n",
        s->id, s->name, s->gender, s->birth, s->major, s->cls);

    Student tmp = *s;  // 创建备份
    char buf[100];
    int chg = 0;  // 修改标志

    // 逐项修改（输入为空则跳过）
    printf("姓名 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) { strcpy(tmp.name, buf); chg = 1; }  // 有输入才修改

    printf("性别 (M/F) [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) {
        char g = (char)toupper(buf[0]);
        if (g != 'M' && g != 'F') {
            printf("性别无效，已跳过。\n");
        }
        else { tmp.gender = g; chg = 1; }
    }

    printf("出生日期 (YYYY-MM-DD) [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) {
        if (!val_date(buf)) {
            printf("日期格式无效，已跳过。\n");
        }
        else { strcpy(tmp.birth, buf); chg = 1; }
    }

    printf("专业 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) { strcpy(tmp.major, buf); chg = 1; }

    printf("班级 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) { strcpy(tmp.cls, buf); chg = 1; }

    if (!chg) {
        printf("未做任何修改。\n");
        return;
    }

    // 验证必要字段
    strcpy(tmp.id, s->id);  // 保持学号不变
    if (!tmp.name[0]) {
        printf("姓名不能为空，修改取消。\n");
        return;
    }
    *s = tmp;  // 保存修改
    printf("学生信息修改成功！\n");
}

// 删除学生功能
static void del_stu(void) {
    char id[MAX_ID];
    printf("请输入要删除的学生学号：");
    getstr(id, sizeof(id));
    int pos = find_stu(id);
    if (pos < 0) {
        printf("未找到该学生！\n");
        return;
    }

    // 外键约束检查：确保没有关联的记录
    for (int i = 0; i < sco_cnt; i++)
        if (strcmp(scores[i].sid, id) == 0) {
            printf("无法删除：该学生有成绩记录！\n");
            return;
        }
    for (int i = 0; i < pay_cnt; i++)
        if (strcmp(payments[i].sid, id) == 0) {
            printf("无法删除：该学生有缴费记录！\n");
            return;
        }

    // 删除学生：将后续元素前移
    for (int i = pos; i < stu_cnt - 1; i++)
        students[i] = students[i + 1];
    stu_cnt--;  // 减少计数
    build_stu_idx();  // 更新索引
    printf("学生删除成功！\n");
}

/* ===== 课程管理功能 ===== */
// 添加课程功能
static void add_cou(void) {
    if (cou_cnt >= MAX_COU) {
        printf("错误：课程数量已达上限！\n");
        return;
    }
    Course c;

    printf("课程代码：");
    getstr(c.code, sizeof(c.code));  // 输入课程代码
    if (!c.code[0]) {
        printf("错误：课程代码不能为空！\n");
        return;
    }
    if (find_cou(c.code) >= 0) {
        printf("错误：课程代码已存在！\n");
        return;
    }

    printf("课程名称：");
    getstr(c.name, sizeof(c.name));  // 输入课程名称
    if (!c.name[0]) {
        printf("错误：课程名称不能为空！\n");
        return;
    }

    printf("学分：");
    if (!getint(&c.credit) || c.credit <= 0 || c.credit > 10) {
        printf("错误：学分必须在1-10之间！\n");
        return;
    }

    printf("学时：");
    if (!getint(&c.hours) || c.hours <= 0) {
        printf("错误：学时无效！\n");
        return;
    }

    courses[cou_cnt++] = c;  // 添加到数组
    build_cou_idx();  // 更新索引
    printf("课程添加成功！\n");
}

// 修改课程功能
static void mod_cou(void) {
    char code[MAX_ID];
    printf("请输入要修改的课程代码：");
    getstr(code, sizeof(code));
    int pos = find_cou(code);
    if (pos < 0) {
        printf("未找到该课程！\n");
        return;
    }

    Course* c = &courses[pos];
    printf("当前信息：%s | %s | %d学分 | %d学时\n",
        c->code, c->name, c->credit, c->hours);

    Course tmp = *c;  // 创建备份
    char buf[100];
    int chg = 0;  // 修改标志

    // 逐项修改
    printf("课程名称 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) { strcpy(tmp.name, buf); chg = 1; }

    printf("学分 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) {
        int v = atoi(buf);
        if (v > 0 && v <= 10) { tmp.credit = v; chg = 1; }
        else printf("无效，已跳过。\n");
    }

    printf("学时 [回车跳过]：");
    getstr(buf, sizeof(buf));
    if (buf[0]) {
        int v = atoi(buf);
        if (v > 0) { tmp.hours = v; chg = 1; }
        else printf("无效，已跳过。\n");
    }

    if (!chg) {
        printf("未做任何修改。\n");
        return;
    }
    strcpy(tmp.code, c->code);  // 保持课程代码不变
    if (!tmp.name[0]) {
        printf("课程名称不能为空，修改取消。\n");
        return;
    }
    *c = tmp;  // 保存修改
    printf("课程修改成功！\n");
}

// 删除课程功能
static void del_cou(void) {
    char code[MAX_ID];
    printf("请输入要删除的课程代码：");
    getstr(code, sizeof(code));
    int pos = find_cou(code);
    if (pos < 0) {
        printf("未找到该课程！\n");
        return;
    }

    // 检查是否有成绩记录关联
    for (int i = 0; i < sco_cnt; i++)
        if (strcmp(scores[i].cid, code) == 0) {
            printf("无法删除：该课程有成绩记录！\n");
            return;
        }

    // 删除课程：数组元素前移
    for (int i = pos; i < cou_cnt - 1; i++)
        courses[i] = courses[i + 1];
    cou_cnt--;
    build_cou_idx();
    printf("课程删除成功！\n");
}

/* ===== 成绩管理功能 ===== */
// 添加成绩功能
static void add_sco(void) {
    if (sco_cnt >= MAX_SCO) {
        printf("错误：成绩数量已达上限！\n");
        return;
    }
    Score s;

    printf("学号：");
    getstr(s.sid, sizeof(s.sid));
    if (find_stu(s.sid) < 0) {
        printf("错误：该学号不存在！\n");
        return;
    }

    printf("课程代码：");
    getstr(s.cid, sizeof(s.cid));
    if (find_cou(s.cid) < 0) {
        printf("错误：该课程代码不存在！\n");
        return;
    }

    // 检查是否已有该学生的该课程成绩
    for (int i = 0; i < sco_cnt; i++)
        if (strcmp(scores[i].sid, s.sid) == 0 && strcmp(scores[i].cid, s.cid) == 0) {
            printf("错误：该学生此课程成绩已存在！\n");
            return;
        }

    // 输入成绩（循环直到有效）
    do {
        printf("成绩 (0-100)：");
    } while (!getfloat(&s.score) || s.score < 0 || s.score > 100);

    printf("学期：");
    if (!getint(&s.semester) || s.semester <= 0) {
        printf("错误：学期无效！\n");
        return;
    }

    scores[sco_cnt++] = s;  // 添加成绩
    printf("成绩添加成功！\n");
}

// 修改成绩功能
static void mod_sco(void) {
    char sid[MAX_ID], cid[MAX_ID];
    printf("学号：");
    getstr(sid, sizeof(sid));
    printf("课程代码：");
    getstr(cid, sizeof(cid));

    // 查找对应的成绩记录
    for (int i = 0; i < sco_cnt; i++) {
        if (strcmp(scores[i].sid, sid) == 0 && strcmp(scores[i].cid, cid) == 0) {
            printf("当前成绩：%.2f  学期：%d\n", scores[i].score, scores[i].semester);
            // 输入新成绩
            do {
                printf("新成绩 (0-100)：");
            } while (!getfloat(&scores[i].score) || scores[i].score < 0 || scores[i].score > 100);
            printf("成绩修改成功！\n");
            return;
        }
    }
    printf("未找到该成绩记录！\n");
}

// 删除成绩功能
static void del_sco(void) {
    char sid[MAX_ID], cid[MAX_ID];
    printf("学号：");
    getstr(sid, sizeof(sid));
    printf("课程代码：");
    getstr(cid, sizeof(cid));

    // 查找并删除成绩记录
    for (int i = 0; i < sco_cnt; i++) {
        if (strcmp(scores[i].sid, sid) == 0 && strcmp(scores[i].cid, cid) == 0) {
            // 数组元素前移
            for (int j = i; j < sco_cnt - 1; j++)
                scores[j] = scores[j + 1];
            sco_cnt--;  // 减少计数
            printf("成绩删除成功！\n");
            return;
        }
    }
    printf("未找到该成绩记录！\n");
}

/* ===== 缴费管理功能 ===== */
// 添加缴费功能
static void add_pay(void) {
    if (pay_cnt >= MAX_PAY) {
        printf("错误：缴费数量已达上限！\n");
        return;
    }
    Payment p;

    printf("缴费编号：");
    getstr(p.pid, sizeof(p.pid));
    if (!p.pid[0]) {
        printf("错误：缴费编号不能为空！\n");
        return;
    }
    // 检查缴费编号是否重复
    for (int i = 0; i < pay_cnt; i++)
        if (strcmp(payments[i].pid, p.pid) == 0) {
            printf("错误：缴费编号已存在！\n");
            return;
        }

    printf("学号：");
    getstr(p.sid, sizeof(p.sid));
    if (find_stu(p.sid) < 0) {
        printf("错误：该学号不存在！\n");
        return;
    }

    printf("金额：");
    if (!getfloat(&p.amount) || p.amount <= 0) {
        printf("错误：金额无效！\n");
        return;
    }

    // 输入并验证缴费日期
    do {
        printf("缴费日期 (YYYY-MM-DD)：");
        getstr(p.date, sizeof(p.date));
    } while (!val_date(p.date));

    payments[pay_cnt++] = p;  // 添加缴费记录
    printf("缴费添加成功！\n");
}

// 删除缴费功能
static void del_pay(void) {
    char pid[MAX_ID];
    printf("请输入要删除的缴费编号：");
    getstr(pid, sizeof(pid));

    // 查找并删除缴费记录
    for (int i = 0; i < pay_cnt; i++) {
        if (strcmp(payments[i].pid, pid) == 0) {
            // 数组元素前移
            for (int j = i; j < pay_cnt - 1; j++)
                payments[j] = payments[j + 1];
            pay_cnt--;  // 减少计数
            printf("缴费删除成功！\n");
            return;
        }
    }
    printf("未找到该缴费记录！\n");
}

/* ===== 信息显示功能 ===== */
// 列出所有学生
static void list_stu(void) {
    // 打印表头
    printf("\n%-12s %-15s %-6s %-12s %-18s %-8s\n", "学号", "姓名", "性别", "出生日期", "专业", "班级");
    printf("---------------------------------------------------------------------------------\n");
    for (int i = 0; i < stu_cnt; i++) {  // 遍历所有学生
        Student* s = &students[i];
        // 格式化输出学生信息
        printf("%-12s %-15s %-6c %-12s %-18s %-8s\n",
            s->id, s->name, s->gender, s->birth, s->major, s->cls);
    }
}

// 列出所有课程
static void list_cou(void) {
    printf("\n%-12s %-25s %-8s %-8s\n", "课程代码", "课程名称", "学分", "学时");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < cou_cnt; i++)
        printf("%-12s %-25s %-8d %-8d\n",
            courses[i].code, courses[i].name, courses[i].credit, courses[i].hours);
}

// 列出所有成绩
static void list_sco(void) {
    printf("\n%-12s %-12s %-10s %-10s\n", "学号", "课程代码", "成绩", "学期");
    printf("---------------------------------------------------\n");
    for (int i = 0; i < sco_cnt; i++)
        printf("%-12s %-12s %-10.2f %-10d\n",
            scores[i].sid, scores[i].cid, scores[i].score, scores[i].semester);
}

// 列出所有缴费
static void list_pay(void) {
    printf("\n%-12s %-12s %-10s %-12s\n", "缴费编号", "学号", "金额", "日期");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < pay_cnt; i++)
        printf("%-12s %-12s %-10.2f %-12s\n",
            payments[i].pid, payments[i].sid, payments[i].amount, payments[i].date);
}

/* ===== 多表关联查询功能 ===== */
// 显示学生-课程-成绩关联信息
static void multi_query(void) {
    printf("\n=== 学生成绩一览 ===\n");
    printf("%-12s %-15s %-18s %-8s\n", "学号", "姓名", "课程名称", "成绩");
    printf("---------------------------------------------------------\n");
    for (int i = 0; i < sco_cnt; i++) {
        int sp = find_stu(scores[i].sid);  // 查找学生
        int cp = find_cou(scores[i].cid);  // 查找课程
        if (sp >= 0 && cp >= 0)  // 如果都找到了
            printf("%-12s %-15s %-18s %-8.2f\n",
                students[sp].id, students[sp].name, courses[cp].name, scores[i].score);
    }

    printf("\n=== 学生缴费一览 ===\n");
    printf("%-12s %-15s %-10s %-12s\n", "学号", "姓名", "金额", "日期");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < pay_cnt; i++) {
        int sp = find_stu(payments[i].sid);  // 查找学生
        if (sp >= 0)  // 如果找到了
            printf("%-12s %-15s %-10.2f %-12s\n",
                students[sp].id, students[sp].name, payments[i].amount, payments[i].date);
    }
}

/* ===== 统计报表功能 ===== */
// 生成统计报告
static void report(void) {
    printf("\n=== 课程成绩汇总 ===\n");
    for (int i = 0; i < cou_cnt; i++) {
        float sum = 0, mx = 0, mn = 100;  // 总分、最高分、最低分
        int cnt = 0;  // 选课人数
        for (int j = 0; j < sco_cnt; j++) {
            if (strcmp(scores[j].cid, courses[i].code) == 0) {  // 找到该课程的成绩
                sum += scores[j].score;
                cnt++;
                if (scores[j].score > mx) mx = scores[j].score;  // 更新最高分
                if (scores[j].score < mn) mn = scores[j].score;  // 更新最低分
            }
        }
        if (cnt > 0)  // 如果有成绩记录
            printf("  %s %-20s 均分:%.2f 最高:%.0f 最低:%.0f 人数:%d\n",
                courses[i].code, courses[i].name, sum / cnt, mx, mn, cnt);
    }

    printf("\n=== 学生缴费汇总 ===\n");
    for (int i = 0; i < stu_cnt; i++) {
        float total = 0;  // 缴费总额
        for (int j = 0; j < pay_cnt; j++)
            if (strcmp(payments[j].sid, students[i].id) == 0)
                total += payments[j].amount;  // 累加缴费金额
        if (total > 0)  // 如果有缴费记录
            printf("  %s %-15s 合计:%.2f\n",
                students[i].id, students[i].name, total);
    }
}

/* ===== 排序功能 ===== */
// 排序比较函数
static int stu_cmp_id(const void* a, const void* b)
{
    return strcmp(((Student*)a)->id, ((Student*)b)->id);
}  // 按学号排序
static int stu_cmp_name(const void* a, const void* b)
{
    return strcmp(((Student*)a)->name, ((Student*)b)->name);
}  // 按姓名排序
static int stu_cmp_birth(const void* a, const void* b)
{
    return strcmp(((Student*)a)->birth, ((Student*)b)->birth);
}  // 按生日排序

// 学生排序功能
static void sort_stu(void) {
    printf("排序方式：1.学号  2.姓名  3.出生日期\n请选择：");
    int ch;
    if (!getint(&ch)) {
        printf("输入无效！\n");
        return;
    }

    int (*cmp)(const void*, const void*) = NULL;  // 函数指针
    if (ch == 1) cmp = stu_cmp_id;
    else if (ch == 2) cmp = stu_cmp_name;
    else if (ch == 3) cmp = stu_cmp_birth;
    else {
        printf("无效选择！\n");
        return;
    }

    qsort(students, stu_cnt, sizeof(Student), cmp);  // 排序
    build_stu_idx();  // 更新索引
    printf("排序完成！\n");
}

/* ===== 条件查找功能 ===== */
// 按姓名或专业查找学生
static void search(void) {
    char kw[MAX_NM];
    printf("请输入查找关键词（姓名或专业）：");
    getstr(kw, sizeof(kw));  // 输入关键词

    // 打印表头
    printf("\n%-12s %-15s %-6s %-12s %-18s %-8s\n", "学号", "姓名", "性别", "出生日期", "专业", "班级");
    printf("---------------------------------------------------------------------------------\n");

    int found = 0;  // 找到标志
    for (int i = 0; i < stu_cnt; i++) {
        Student* s = &students[i];
        // 检查姓名或专业是否包含关键词
        if (strstr(s->name, kw) || strstr(s->major, kw)) {
            printf("%-12s %-15s %-6c %-12s %-18s %-8s\n",
                s->id, s->name, s->gender, s->birth, s->major, s->cls);
            found = 1;  // 标记找到
        }
    }
    if (!found) printf("未找到匹配的学生。\n");
}

/* ===== 索引展示功能 ===== */
// 显示当前索引状态
static void show_idx(void) {
    printf("\n=== 学生索引（按学号） ===\n");
    printf("%-12s %-6s\n", "键值", "位置");
    printf("--------------------\n");
    for (int i = 0; i < stu_ic; i++)
        printf("%-12s %-6d\n", stu_idx[i].key, stu_idx[i].pos);

    printf("\n=== 课程索引（按课程代码） ===\n");
    printf("%-12s %-6s\n", "键值", "位置");
    printf("--------------------\n");
    for (int i = 0; i < cou_ic; i++)
        printf("%-12s %-6d\n", cou_idx[i].key, cou_idx[i].pos);

    printf("\n通过排序索引实现 O(log n) 二分查找。\n");
}

/* ===== 初始化数据文件功能 ===== */
// 初始化所有数据文件
static void init_files(void) {
    stu_cnt = cou_cnt = sco_cnt = pay_cnt = 0;  // 清零所有计数器
    save_all();  // 保存空数据到文件
    build_idx();  // 建立空索引
    printf("数据文件初始化完成！\n");
}

/* ===== 主菜单功能 ===== */
// 显示主菜单
static void menu(void) {
    printf("\n=========== 学生信息管理系统 ===========\n");
    printf("  1.添加学生        2.修改学生\n");
    printf("  3.删除学生        4.学生列表\n");
    printf("  5.添加课程        6.修改课程\n");
    printf("  7.删除课程        8.课程列表\n");
    printf("  9.添加成绩       10.修改成绩\n");
    printf(" 11.删除成绩       12.成绩列表\n");
    printf(" 13.添加缴费       14.删除缴费\n");
    printf(" 15.缴费列表       16.多表关联查询\n");
    printf(" 17.统计报表       18.学生排序\n");
    printf(" 19.条件查找       20.索引展示\n");
    printf(" 21.初始化文件      0.退出系统\n");
    printf("========================================\n");
    printf("请选择：");
}

/* ===== 主函数 ===== */
int main(void) {
    load_all();     // 启动时加载所有数据
    build_idx();    // 建立索引

    int ch;  // 用户选择
    while (1) {  // 主循环
        menu();  // 显示菜单
        if (!getint(&ch)) {  // 读取用户输入
            printf("输入无效，请重新输入！\n");
            continue;
        }
        // 根据选择执行相应功能
        switch (ch) {
        case  1: add_stu();      break;  // 添加学生
        case  2: mod_stu();      break;  // 修改学生
        case  3: del_stu();      break;  // 删除学生
        case  4: list_stu();     break;  // 学生列表
        case  5: add_cou();      break;  // 添加课程
        case  6: mod_cou();      break;  // 修改课程
        case  7: del_cou();      break;  // 删除课程
        case  8: list_cou();     break;  // 课程列表
        case  9: add_sco();      break;  // 添加成绩
        case 10: mod_sco();      break;  // 修改成绩
        case 11: del_sco();      break;  // 删除成绩
        case 12: list_sco();     break;  // 成绩列表
        case 13: add_pay();      break;  // 添加缴费
        case 14: del_pay();      break;  // 删除缴费
        case 15: list_pay();     break;  // 缴费列表
        case 16: multi_query();  break;  // 多表查询
        case 17: report();       break;  // 统计报表
        case 18: sort_stu();     break;  // 学生排序
        case 19: search();       break;  // 条件查找
        case 20: show_idx();     break;  // 索引展示
        case 21: init_files();   break;  // 初始化文件
        case  0: save_all(); printf("再见！\n"); return 0;  // 退出
        default: printf("无效选择，请重新输入！\n");  // 无效选择
        }
    }
}

