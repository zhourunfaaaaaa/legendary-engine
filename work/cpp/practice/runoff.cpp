// 这个runoff算法的关键点：1.选出最终结果的条件是超过半数的投票
//                       2.平局的条件极为严格：所有人的票数都不超过一半，并且所有剩下的人票数相等
//                       3.每一轮票数都是固定的
//                       4.新一轮候选人的票数一定不会低于前一轮
//                       5.每一次有关票数的检测都要判断他是否已经淘汰
//                       6.每次淘汰一个或多个最小票数的人，但在这之前要判断是否都为最小票数，也就是判断是否相等
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// 定义最大限制
#define MAX_VOTERS 100
#define MAX_CANDIDATES 9

// 核心数据结构：候选人
typedef struct {
    char* name;    // 名字（指向命令行参数的指针）
    int votes;     // 当前轮次的得票数
    bool isOut;    // 状态标记：是否已出局（算法的关键阀门）
} candidate;

// 全局变量：存储选举状态
candidate candidates[MAX_CANDIDATES];
int preferences[MAX_VOTERS][MAX_CANDIDATES]; // 偏好矩阵：存储的是候选人的编号索引
int candidate_cnt;
int voter_cnt;

// 函数原型
bool vote(int voter, int rank, char *name);
void tabulate();
bool print_winner();
int find_min();
bool is_tie(int min);
void eliminate(int min);

int main(int argc, char* argv[])
{
    // 初始化候选人信息
    candidate_cnt = argc - 1;
    printf("Number of voter?: ");
    scanf("%d", &voter_cnt);
    for (int i = 0; i < candidate_cnt; i++) {
        candidates[i].name = argv[i+1];
        candidates[i].isOut = false; // 初始所有人都在场
    }

    // 投票阶段：构建偏好矩阵
    for (int i = 0; i < voter_cnt; i++) {
        for (int j = 0; j < candidate_cnt; j++) {
            char name[50]; // 【易错点】：必须分配内存空间，不能用未初始化的指针
            printf("Rank %d: ", j+1);
            do {
                scanf("%s", name);
            } while (!vote(i, j , name)); // 【逻辑点】：直到输入正确的名字才继续
        }
    }

    // 核心算法循环：每一轮就是一个“过滤”过程
    while (1) {
        // 1. 重新计票（票数流动的单调性体现）
        tabulate();
        
        // 2. 检查是否有赢家（超过50%的绝对多数）
        if (print_winner()) {
            break; // 产生赢家，跳出循环，程序结束
        } 
        
        // 3. 没赢家，则寻找当前场上的最低票数
        int min = find_min();
        
        // 4. 判定是否为平局（死局判断）
        if (!is_tie(min)) {
            // 如果不是平局，说明有人高有人低，踢掉最低的
            eliminate(min);
        } else {
            // 如果是平局，说明剩下的人不分胜负，全员胜出，胜出的人都没有被淘汰，所以直接输出没有被淘汰的人就行
            for (int i = 0; i < candidate_cnt; i++) {
                if (!candidates[i].isOut) {
                    printf("%s\n", candidates[i].name);
                }
            }
            break; // 平局也是终点，跳出循环
        }
    }

    return 0;
}

// 投票：将名字转化为索引存入矩阵
bool vote(int voter, int rank, char *name)
{
    for (int i = 0; i < candidate_cnt; i++) {
        if (strcmp(name, candidates[i].name) == 0) { // 【易错点】：C语言字符串比较必须用strcmp
            preferences[voter][rank] = i; // 存入编号，实现数据的抽象化，可以通过这个编号作为下表，直接访问candidate这个结构体了，不需要再比较字符串
            return true;
        }
    }
    return false;
}

// 计票：模拟选票的“接力”
void tabulate()
{
    // 【易错点】：每轮计票前必须清零，否则得票数会单调递增导致逻辑错误
    for (int i = 0; i < candidate_cnt; i++) {
        candidates[i].votes = 0;
    }
    
    // 遍历每个选民
    for (int i = 0; i < voter_cnt; i++) {
        // 顺着该选民的偏好表往下找
        for (int j = 0; j < candidate_cnt; j++) {
            int candidate_idx = preferences[i][j];
            // 【核心逻辑】：只投给还没出局的最高顺位候选人
            if (!candidates[candidate_idx].isOut) {
                candidates[candidate_idx].votes++;
                break; // 该选民这一轮的任务完成，跳出j循环看下一个选民i
            }
        }
    }
}

// 判定赢家
bool print_winner()
{
    for (int i = 0; i < candidate_cnt; i++) {
        // 【逻辑点】：必须严格大于总票数的一半，体现票数守恒下的绝对多数
        if (candidates[i].votes > voter_cnt/2) {
            printf("%s\n", candidates[i].name);
            return true;
        }
    }
    return false;
}

// 寻找当前场上的最小值
int find_min()
{
    // 【易错点】：初始值应设为可能的最大票数，而非0
    int min_val = voter_cnt;
    for (int i = 0; i < candidate_cnt; i++) { // 建议从0开始遍历更严谨
        // 【核心逻辑】：必须过滤掉已经出局的人，否则min永远会被锁定在“死人”的0票上
        if (!candidates[i].isOut && candidates[i].votes < min_val) {
            min_val = candidates[i].votes;
        }
    }
    return min_val;
}

// 判断平局
bool is_tie(int min)
{
    for (int i = 0; i < candidate_cnt; i++) {
        if (!candidates[i].isOut) {
            // 【逻辑点】：只要场上有一个活人的票数不等于最小值，就不是平局
            if (candidates[i].votes != min) {
                return false;
            }
        }
    }
    // 所有活着的候选人票数全部等于min，说明难分伯仲
    return true;
}

// 淘汰
void eliminate(int min)
{
    for (int i = 0; i < candidate_cnt; i++) {
        // 【核心逻辑】：淘汰所有得票数为min的人，哪怕有多个人
        if (!candidates[i].isOut && candidates[i].votes == min) {
            candidates[i].isOut = true; // 关掉阀门
        } 
    }
}