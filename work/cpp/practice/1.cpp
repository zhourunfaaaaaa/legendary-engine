#include <graphics.h>
#include <conio.h>

int main() {
    // 初始化一个 800 x 600 的绘图窗口
    initgraph(800, 600);
    
    // 设置背景色为白色，并清空屏幕
    setbkcolor(WHITE);
    cleardevice();
    
    // 设置绘图颜色为黑色（用于画点和线）
    setlinecolor(BLACK);
    setfillcolor(BLACK);

    POINT pts[3];       // 用于存储三个鼠标点击的坐标点
    int clickCount = 0; // 记录当前点击的次数
    ExMessage msg;      // 定义消息变量，用于接收鼠标和键盘消息

    // 提示信息
    settextcolor(BLACK);
    outtextxy(10, 10, _T("请用鼠标左键在窗口中点击三个位置。按 ESC 键退出。"));

    while (true) {
        // 获取消息（包括鼠标和键盘消息）
        msg = getmessage(EX_MOUSE | EX_KEY);

        // 如果按下了 ESC 键，退出程序
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
            break;
        }

        // 处理鼠标左键按下消息
        if (msg.message == WM_LBUTTONDOWN) {
            // 如果已经画了三角形，再次点击时清空屏幕，准备画下一个
            if (clickCount == 3) {
                cleardevice();
                outtextxy(10, 10, _T("请用鼠标左键在窗口中点击三个位置。按 ESC 键退出。"));
                clickCount = 0; 
            }

            // 记录当前鼠标点击的 x 和 y 坐标
            pts[clickCount].x = msg.x;
            pts[clickCount].y = msg.y;

            // 在点击的位置画一个小实心圆，给用户视觉反馈
            solidcircle(msg.x, msg.y, 3);

            clickCount++; // 点击次数加 1

            // 当收集到 3 个点时，绘制三角形
            if (clickCount == 3) {
                // 方法 1：直接用线连接三个点
                line(pts[0].x, pts[0].y, pts[1].x, pts[1].y);
                line(pts[1].x, pts[1].y, pts[2].x, pts[2].y);
                line(pts[2].x, pts[2].y, pts[0].x, pts[0].y);

                // 方法 2：也可以直接使用 EasyX 的多边形函数（取消下面这行的注释即可替换线段绘制）
                // polygon(pts, 3);
            }
        }
    }

    // 关闭绘图窗口
    closegraph();
    return 0;
}