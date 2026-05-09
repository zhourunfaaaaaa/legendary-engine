#include <graphics.h>
#include <conio.h>

// 定义常量，方便后续调整 UI 布局
const int CELL_SIZE = 40;                     // 每个格子的边长 (像素)
const int COLS = 16;                          // 网格列数
const int ROWS = 8;                           // 网格行数
const int GRID_HEIGHT = ROWS * CELL_SIZE;     // 绘图区总高度 (320)
const int WINDOW_WIDTH = COLS * CELL_SIZE;    // 窗口宽度 (640)
const int WINDOW_HEIGHT = GRID_HEIGHT + 100;  // 窗口高度 (增加100像素给底部调色板)

// 定义 8 种常用颜色作为画笔
const int PALETTE_SIZE = 8;
COLORREF palette[PALETTE_SIZE] = { 
    RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, BLACK, WHITE 
};

// 全局变量：存储16x8网格每一个格子的颜色
COLORREF grid[ROWS][COLS];
// 当前选中的画笔颜色（默认选红色）
COLORREF currentColor = RED;

// 绘制单个网格
void drawCell(int r, int c) {
    setfillcolor(grid[r][c]);
    setlinecolor(LIGHTGRAY); // 网格线用浅灰色
    setlinestyle(PS_SOLID, 1);
    
    int left = c * CELL_SIZE;
    int top = r * CELL_SIZE;
    fillrectangle(left, top, left + CELL_SIZE, top + CELL_SIZE);
}

// 绘制整个网格区
void drawAllCells() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            drawCell(r, c);
        }
    }
}

// 绘制底部调色板
void drawPalette() {
    // 调色板区域背景设为浅灰色以区分绘图区
    setfillcolor(RGB(240, 240, 240));
    solidrectangle(0, GRID_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 提示文字
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    outtextxy(10, GRID_HEIGHT + 10, _T("选择颜色:"));

    // 计算调色板色块的起始 X 坐标，使其居中显示
    // 每个色块宽 40，间隔 10，总宽度为 8*40 + 7*10 = 390
    int startX = (WINDOW_WIDTH - 390) / 2;
    int startY = GRID_HEIGHT + 40;

    for (int i = 0; i < PALETTE_SIZE; i++) {
        int px = startX + i * 50; // 每个色块占 40 宽度 + 10 间距

        setfillcolor(palette[i]);
        
        // 如果是当前选中的颜色，画一个粗黑边框；否则画细灰边框
        if (currentColor == palette[i]) {
            setlinecolor(BLACK);
            setlinestyle(PS_SOLID, 3); 
        } else {
            setlinecolor(LIGHTGRAY);
            setlinestyle(PS_SOLID, 1);
        }
        
        fillrectangle(px, startY, px + 40, startY + 40);
    }
}

int main() {
    // 初始化窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(WHITE);
    cleardevice();
    
    // 初始化网格数组，默认全部为白色
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c] = WHITE;
        }
    }

    // 初次绘制界面
    drawAllCells();
    drawPalette();

    ExMessage msg;
    while (true) {
        msg = getmessage(EX_MOUSE | EX_KEY);

        // 按 ESC 键退出
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
            break;
        }

        // 处理鼠标左键按下事件
        if (msg.message == WM_LBUTTONDOWN) {
            int mx = msg.x;
            int my = msg.y;

            // 1. 判断是否点击了上半部分的“绘图区”
            if (my < GRID_HEIGHT) {
                // 根据坐标反推点击了第几行第几列
                int c = mx / CELL_SIZE;
                int r = my / CELL_SIZE;
                
                // 边界安全检查（防止数组越界）
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    grid[r][c] = currentColor; // 更新数据
                    drawCell(r, c);            // 仅重绘被点击的这一个格子，提高效率
                }
            }
            // 2. 判断是否点击了下半部分的“调色板”区 (Y坐标在 startY 到 startY+40 之间)
            else if (my >= GRID_HEIGHT + 40 && my <= GRID_HEIGHT + 80) {
                int startX = (WINDOW_WIDTH - 390) / 2;
                
                // 遍历8个色块，看看点击的X坐标落在了哪个色块内
                for (int i = 0; i < PALETTE_SIZE; i++) {
                    int px = startX + i * 50;
                    if (mx >= px && mx <= px + 40) {
                        currentColor = palette[i]; // 切换当前颜色
                        drawPalette();             // 重绘底部区域（更新选中框的样式）
                        break;
                    }
                }
            }
        }
    }

    closegraph();
    return 0;
}