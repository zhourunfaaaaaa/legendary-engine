//汉诺塔
#include <stdio.h>

void hanoi(int n, char source, char auxiliary, char target) {
    if (n == 1) {
        printf("%c to %c\n", source, target);
        return;
    }
    
    // 第一步：把上面 n-1 个盘子移到辅助柱
    hanoi(n - 1, source, target, auxiliary);
    
    // 第二步：把最大的盘子移到目标柱  
    printf("%c to %c\n", source, target);
    
    // 第三步：把 n-1 个盘子从辅助柱移到目标柱
    hanoi(n - 1, auxiliary, source, target);
}

int main() {
    hanoi(2, 'S', 'A', 'T');  // 把2个盘子从S移到T，借助A
    return 0;
}