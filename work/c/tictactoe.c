#include <stdio.h>

int main() {
    const int size = 3;
    int board[size][size];
    int i,j;
    int numofO,numofX;
    int result = -1;//-1:没人赢 1:X赢 0:O赢

    //读入矩阵
    for (i = 0;i < size;i++) {
        for (j = 0;j < size;j++)
        printf("请输入第%d行，第%d列的棋子。",i,j);
        scanf("%d",&board[i][j]);
    }

    //检查行
    for (i = 0 ;i < size && result == -1;i++) {
        numofO = numofX = 0;
        for (j = 0;j < size;j++) {
            if (board[i][j] == 1) {
                numofX++;
            } else {
                numofO++;
            }
        }
        if (numofO == 3) {
            result = 0;
        } else if (numofX == 3) {
            result = 1;
        }
    }
    //检查列
    if (result == -1) {
        for (j = 0 ;j < size;j++) {
            numofO = numofX = 0;
            for (i = 0;i < size;i++) {
                if (board[i][j] == 1) {
                numofX++;
                } else {
                numofO++;
                }
            }
            if (numofO == 3) {
                result = 0;
            } else if (numofX == 3) {
                result = 1;
            } 
        }
    }
    //检查对角线
    if (result == -1) {
        numofO = numofX = 0;
        for (i = 0;i < size;i++) {
            if (board[i][i] == 1) {
                numofX++;
            } else {
                numofO++;
            }
        }
        if (numofO == 3) {
                result = 0;
            } else if (numofX == 3) {
                result = 1;
            }
    }
    if (result == -1) {
        for (i = 0;i < size;i++) {
            if (board[i][size-i-1] == 1) {
                numofX++;
            } else {
                numofO++;
            }
        }
        if (numofO == 3) {
                result = 0;
            } else if (numofX == 3) {
                result = 1;
            }
    }
    return 0;
}