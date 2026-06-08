#include <graphics.h>
#include <conio.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>

const int WIN_W = 640;
const int WIN_H = 480;
const int CARD_W = 120;
const int CARD_H = 120;
const int GAP = 12;
const int GRID_COLS = 3;
const int GRID_ROWS = 3;
const int GRID_TOTAL_W = GRID_COLS * CARD_W + (GRID_COLS - 1) * GAP;
const int GRID_TOTAL_H = GRID_ROWS * CARD_H + (GRID_ROWS - 1) * GAP;
const int GRID_X = (WIN_W - GRID_TOTAL_W) / 2;
const int GRID_Y = (WIN_H - GRID_TOTAL_H) / 2;
const int MAX_CLICKS = 5;

struct Card {
    int x, y;
    int w, h;
    int type;
    bool revealed;
};

Card cards[9];
IMAGE faces[3];
int clickCount;
bool gameOver;
bool gameWin;
int hoveredCard;

void initFaces() {
    const char* filenames[] = {"photo0.jpg", "photo1.jpg", "photo2.jpg"};
    COLORREF colors[] = {RGB(220, 70, 70), RGB(70, 130, 220), RGB(70, 190, 90)};
    for (int t = 0; t < 3; t++) {
        IMAGE img(CARD_W, CARD_H);
        bool loaded = false;
        FILE* test = fopen(filenames[t], "rb");
        if (test) {
            fclose(test);
            loadimage(&img, filenames[t], CARD_W, CARD_H);
            loaded = true;
        }
        if (!loaded) {
            SetWorkingImage(&img);
            setfillcolor(colors[t]);
            solidrectangle(0, 0, CARD_W - 1, CARD_H - 1);
            setfillcolor(WHITE);
            if (t == 0) {
                solidcircle(CARD_W / 2, CARD_H / 2, 40);
                setfillcolor(colors[t]);
                solidcircle(CARD_W / 2, CARD_H / 2, 18);
            } else if (t == 1) {
                POINT tri[] = {{CARD_W / 2, 16}, {16, CARD_H - 16}, {CARD_W - 16, CARD_H - 16}};
                solidpolygon(tri, 3);
                setfillcolor(colors[t]);
                POINT tri2[] = {{CARD_W / 2, 42}, {42, CARD_H - 42}, {CARD_W - 42, CARD_H - 42}};
                solidpolygon(tri2, 3);
            } else {
                solidrectangle(18, 18, CARD_W - 18, CARD_H - 18);
                setfillcolor(colors[t]);
                solidrectangle(38, 38, CARD_W - 38, CARD_H - 38);
                setfillcolor(WHITE);
                solidrectangle(52, 52, CARD_W - 52, CARD_H - 52);
            }
            SetWorkingImage();
        }
        faces[t] = img;
    }
}

void initCards() {
    for (int i = 0; i < 9; i++) {
        cards[i].type = i % 3;
        cards[i].revealed = false;
    }
    for (int i = 8; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = cards[i].type;
        cards[i].type = cards[j].type;
        cards[j].type = tmp;
    }
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int idx = r * GRID_COLS + c;
            cards[idx].x = GRID_X + c * (CARD_W + GAP);
            cards[idx].y = GRID_Y + r * (CARD_H + GAP);
            cards[idx].w = CARD_W;
            cards[idx].h = CARD_H;
        }
    }
}

void drawCardBack(int x, int y, bool hovered) {
    int br = hovered ? 95 : 55;
    int bg = hovered ? 95 : 55;
    int bb = hovered ? 120 : 75;
    setfillcolor(RGB(br, bg, bb));
    solidrectangle(x, y, x + CARD_W, y + CARD_H);
    setfillcolor(RGB(br + 20, bg + 20, bb + 20));
    solidrectangle(x + 5, y + 5, x + CARD_W - 5, y + CARD_H - 5);
    setfillcolor(RGB(br + 5, bg + 5, bb + 5));
    solidrectangle(x + 8, y + 8, x + CARD_W - 8, y + CARD_H - 8);
    setfillcolor(RGB(br + 30, bg + 30, bb + 30));
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            int cx = x + 28 + c * 32;
            int cy = y + 28 + r * 32;
            solidcircle(cx, cy, 7);
        }
    }
}

void drawCard(int idx) {
    Card& c = cards[idx];
    if (c.revealed) {
        putimage(c.x, c.y, &faces[c.type]);
    } else {
        drawCardBack(c.x, c.y, idx == hoveredCard);
    }
}

void drawGame() {
    cleardevice();
    setbkcolor(RGB(25, 25, 38));
    cleardevice();

    settextcolor(RGB(200, 200, 220));
    settextstyle(26, 0, _T("Microsoft YaHei"));
    setbkmode(TRANSPARENT);
    RECT tR = {0, 6, WIN_W, 40};
    drawtext(_T("记忆翻牌"), &tR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(150, 150, 170));
    settextstyle(15, 0, _T("Microsoft YaHei"));
    RECT sR = {GRID_X, GRID_Y - 28, GRID_X + GRID_TOTAL_W, GRID_Y - 3};
    drawtext(_T("点击 5 个格子，找出 3 张相同的图片"), &sR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    char buf[32];
    sprintf(buf, "剩余点击: %d", MAX_CLICKS - clickCount);
    settextcolor(RGB(180, 180, 200));
    settextstyle(18, 0, _T("Microsoft YaHei"));
    RECT cR = {0, GRID_Y + GRID_TOTAL_H + 8, WIN_W, GRID_Y + GRID_TOTAL_H + 34};
    drawtext(buf, &cR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (gameOver) {
        for (int i = 0; i < 9; i++) {
            putimage(cards[i].x, cards[i].y, &faces[cards[i].type]);
        }
    } else {
        for (int i = 0; i < 9; i++) drawCard(i);
    }
}

void drawWinScreen() {
    int fx = 130, fy = 75, fw = 380, fh = 330;

    setfillcolor(RGB(10, 15, 45));
    solidrectangle(fx, fy, fx + fw, fy + fh);

    setlinecolor(RGB(255, 215, 0));
    setlinestyle(PS_SOLID, 4);
    rectangle(fx, fy, fx + fw, fy + fh);
    setlinestyle(PS_SOLID, 1);
    rectangle(fx + 8, fy + 8, fx + fw - 8, fy + fh - 8);

    setfillcolor(RGB(255, 215, 0));
    for (int i = 0; i < 18; i++) {
        int sx = fx + 25 + rand() % (fw - 50);
        int sy = fy + 25 + rand() % (fh - 50);
        int sz = 2 + rand() % 6;
        solidcircle(sx, sy, sz);
    }

    settextcolor(RGB(255, 215, 0));
    settextstyle(50, 0, _T("Microsoft YaHei"));
    setbkmode(TRANSPARENT);
    RECT wR = {fx, fy + 55, fx + fw, fy + 130};
    drawtext(_T("胜利！"), &wR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(220, 220, 255));
    settextstyle(20, 0, _T("Microsoft YaHei"));
    RECT subR = {fx, fy + 155, fx + fw, fy + 190};
    drawtext(_T("成功找到 3 张相同的图片"), &subR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(160, 160, 180));
    settextstyle(15, 0, _T("Microsoft YaHei"));
    RECT resR = {fx, fy + 240, fx + fw, fy + 268};
    drawtext(_T("点击任意位置重新开始 | ESC 退出"), &resR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void drawLoseScreen() {
    int fx = 130, fy = 75, fw = 380, fh = 330;

    setfillcolor(RGB(38, 12, 12));
    solidrectangle(fx, fy, fx + fw, fy + fh);

    setlinecolor(RGB(210, 55, 55));
    setlinestyle(PS_SOLID, 4);
    rectangle(fx, fy, fx + fw, fy + fh);
    setlinestyle(PS_SOLID, 1);
    rectangle(fx + 8, fy + 8, fx + fw - 8, fy + fh - 8);

    setfillcolor(RGB(210, 70, 70));
    for (int i = 0; i < 10; i++) {
        int sx = fx + 25 + rand() % (fw - 50);
        int sy = fy + 25 + rand() % (fh - 50);
        int sz = 2 + rand() % 5;
        solidcircle(sx, sy, sz);
    }

    settextcolor(RGB(255, 80, 80));
    settextstyle(50, 0, _T("Microsoft YaHei"));
    setbkmode(TRANSPARENT);
    RECT lR = {fx, fy + 55, fx + fw, fy + 130};
    drawtext(_T("失败"), &lR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(210, 160, 160));
    settextstyle(20, 0, _T("Microsoft YaHei"));
    RECT subR = {fx, fy + 155, fx + fw, fy + 190};
    drawtext(_T("5 次点击未找到 3 张相同图片"), &subR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(150, 150, 160));
    settextstyle(15, 0, _T("Microsoft YaHei"));
    RECT resR = {fx, fy + 240, fx + fw, fy + 268};
    drawtext(_T("点击任意位置重新开始 | ESC 退出"), &resR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void checkWin() {
    if (clickCount >= MAX_CLICKS) {
        int cnt[3] = {0};
        for (int i = 0; i < 9; i++) {
            if (cards[i].revealed) cnt[cards[i].type]++;
        }
        gameOver = true;
        gameWin = (cnt[0] >= 3 || cnt[1] >= 3 || cnt[2] >= 3);
    }
}

void resetGame() {
    clickCount = 0;
    gameOver = false;
    gameWin = false;
    hoveredCard = -1;
    initCards();
}

int main() {
    srand((unsigned)time(NULL));
    initgraph(WIN_W, WIN_H);
    setbkcolor(RGB(25, 25, 38));

    initFaces();
    resetGame();

    BeginBatchDraw();

    while (true) {
        drawGame();

        if (gameOver) {
            if (gameWin) drawWinScreen();
            else drawLoseScreen();
        }

        FlushBatchDraw();

        if (_kbhit()) {
            if (_getch() == 27) break;
        }

        MOUSEMSG m;
        if (MouseHit()) {
            m = GetMouseMsg();

            if (m.uMsg == WM_MOUSEMOVE && !gameOver) {
                hoveredCard = -1;
                for (int i = 0; i < 9; i++) {
                    if (!cards[i].revealed &&
                        m.x >= cards[i].x && m.x <= cards[i].x + CARD_W &&
                        m.y >= cards[i].y && m.y <= cards[i].y + CARD_H) {
                        hoveredCard = i;
                        break;
                    }
                }
            }

            if (m.uMsg == WM_LBUTTONDOWN) {
                if (gameOver) {
                    resetGame();
                    continue;
                }
                for (int i = 0; i < 9; i++) {
                    if (!cards[i].revealed &&
                        m.x >= cards[i].x && m.x <= cards[i].x + CARD_W &&
                        m.y >= cards[i].y && m.y <= cards[i].y + CARD_H) {
                        cards[i].revealed = true;
                        clickCount++;
                        checkWin();
                        break;
                    }
                }
            }
        }

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
