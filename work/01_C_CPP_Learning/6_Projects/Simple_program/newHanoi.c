#include <stdio.h>

int step = 0;  // 记录总步数

/**********Begin**********/
// 修改版汉诺塔移动函数
void hanoi_modified(int n, char from, char aux, char to) {
    if (n == 0) return;
    
    if (n == 1) {
        // 只有一个盘子
        if ((from == 'A' && to == 'C') || (from == 'C' && to == 'A')) {
            // 不能直接从A到C或C到A，需要两步
            step++;
            printf("第%2d步: 将盘子 %d 从 %c 移动到 %c\n", step, 1, from, aux);
            step++;
            printf("第%2d步: 将盘子 %d 从 %c 移动到 %c\n", step, 1, aux, to);
        } else {
            // 可以直接移动
            step++;
            printf("第%2d步: 将盘子 %d 从 %c 移动到 %c\n", step, 1, from, to);
        }
        return;
    }
    
    // 步骤1：先把上面n-1个盘子从from移到to（遵守规则）
    hanoi_modified(n-1, from, aux, to);
    
    // 步骤2：把最大的盘子从from移到aux（因为不能直接从from到to）
    step++;
    printf("第%2d步: 将盘子 %d 从 %c 移动到 %c\n", step, n, from, aux);
    
    // 步骤3：把n-1个盘子从to移回from（遵守规则）
    hanoi_modified(n-1, to, aux, from);
    
    // 步骤4：把最大的盘子从aux移到to
    step++;
    printf("第%2d步: 将盘子 %d 从 %c 移动到 %c\n", step, n, aux, to);
    
    // 步骤5：把n-1个盘子从from移到to（遵守规则）
    hanoi_modified(n-1, from, aux, to);
}

int main() {
    int n;
    scanf("%d", &n);
    
    step = 0;
    printf("移动 %d 个盘子的过程:\n", n);
    hanoi_modified(n, 'A', 'B', 'C');
    
    printf("\n总共需要 %d 步\n", step);
    
    return 0;
}