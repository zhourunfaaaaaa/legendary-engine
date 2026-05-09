// ============================================================
// Knight's Journey - 2D 俯视角弹幕射击 Roguelike
// 技术栈：C++11, EasyX Graphics Library, Windows API
// ============================================================

#include <windows.h>
#include <graphics.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "include/Core/Common.h"
#include "include/Core/GameManager.h"

// ============================================================
// WinMain - 程序入口点
// ============================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    // ---- EasyX 窗口初始化 ----
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EW_SHOWCONSOLE);
    HWND hWnd = GetHWnd();
    SetWindowText(hWnd, TEXT("Knight's Journey - Legendary Engine"));

    // 设置窗口居中
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    RECT winRect;
    GetWindowRect(hWnd, &winRect);
    int winW = winRect.right - winRect.left;
    int winH = winRect.bottom - winRect.top;
    SetWindowPos(hWnd, HWND_TOP,
                 (screenW - winW) / 2,
                 (screenH - winH) / 2,
                 0, 0, SWP_NOSIZE);

    // 控制台标题
    SetConsoleTitle(TEXT("Knight's Journey - Debug Console"));

    // EasyX 渲染预设
    setbkcolor(RGB(26, 26, 46));    // 深色背景
    setbkmode(TRANSPARENT);
    settextcolor(0xFFFFFF);
    cleardevice();

    // 启用双缓冲绘制
    BeginBatchDraw();

    // ---- 游戏单例初始化 ----
    GameManager& game = GameManager::GetInstance();
    game.Init();

    // ---- 主循环 ----
    LARGE_INTEGER freq, frameStart, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&frameStart);

    MSG msg = { 0 };
    bool running = true;

    while (running) {
        // 1. 处理 Windows 消息队列（防止窗口卡死）
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 2. 计算本帧 deltaTime（距上一帧开始的秒数）
        QueryPerformanceCounter(&now);
        float deltaTime = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                        / freq.QuadPart;
        QueryPerformanceCounter(&frameStart);  // 重置计时起点

        // 帧率尖刺保护：单帧最多模拟 100ms，超出则截断
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        if (deltaTime <= 0.0f) deltaTime = 0.016f;  // 安全兜底

        // 3. 委托 GameManager 处理完整一帧
        game.ProcessFrame(deltaTime);

        // 4. 帧率限制（目标 60 FPS）
        const float targetFrameTime = 1.0f / 60.0f;
        QueryPerformanceCounter(&now);
        float elapsed = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                      / freq.QuadPart;
        if (elapsed < targetFrameTime) {
            // Sleep 粗略等待（-1ms 避免睡过头）
            float waitTime = targetFrameTime - elapsed;
            DWORD sleepMs = static_cast<DWORD>(waitTime * 1000.0f);
            if (sleepMs > 1) {
                Sleep(sleepMs - 1);
            }
            // busy-wait 微调至精确时间点
            do {
                QueryPerformanceCounter(&now);
                elapsed = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                        / freq.QuadPart;
            } while (elapsed < targetFrameTime);
        }
    }

    // ---- 清理 ----
    game.Shutdown();
    EndBatchDraw();
    closegraph();

    return 0;
}

// ============================================================
// 编译说明
// ============================================================
// EasyX 库安装: https://easyx.cn/
//
// MSVC (VS Developer Command Prompt):
//   cl /EHsc /std:c++11 /I. main.cpp src\*.cpp /Fe:KnightsJourney.exe
//
// MinGW-w64:
//   g++ -std=c++11 -I. main.cpp src/*.cpp -lgraphics -lgdi32 -lmsimg32 -lwinmm
//       -o KnightsJourney.exe
// ============================================================
