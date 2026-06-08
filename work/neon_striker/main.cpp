// ============================================================
// Neon Striker - 2D 赛博朋克弹幕射击
// 技术栈：C++11, EasyX Graphics Library, Windows API
// 从 neon_striker_v2.html 改写
// ============================================================

#include <windows.h>
#include <graphics.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "Common.h"
#include "GameManager.h"
#include "Input.h"
#include "Renderer.h"

// ============================================================
// WinMain - 程序入口点
// ============================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    // ---- EasyX 窗口初始化 ----
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EW_SHOWCONSOLE);
    HWND hWnd = GetHWnd();
    SetWindowText(hWnd, TEXT("Neon Striker - Legendary Engine"));

    // 窗口居中
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

    SetConsoleTitle(TEXT("Neon Striker - Debug Console"));

    // EasyX 渲染预设
    setbkcolor(RGB(8, 8, 30));
    setbkmode(TRANSPARENT);
    settextcolor(0xFFFFFF);
    cleardevice();

    // 启用双缓冲
    BeginBatchDraw();

    // 初始化
    Input::GetInstance().Init();
    GameManager& game = GameManager::GetInstance();
    game.Init();
    Renderer::GetInstance().Init();

    // 加载最高分
    game.SetHighScore(GetProfileIntA("NeonStriker", "HighScore", 0));

    // ---- 主循环 ----
    LARGE_INTEGER freq, frameStart, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&frameStart);

    MSG msg = { 0 };
    bool running = true;

    while (running) {
        // 1. 处理 Windows 消息
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 2. 计算 deltaTime
        QueryPerformanceCounter(&now);
        float rawDt = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                    / freq.QuadPart;
        QueryPerformanceCounter(&frameStart);

        // 帧率尖刺保护
        if (rawDt > MAX_FRAME_TIME) rawDt = MAX_FRAME_TIME;
        if (rawDt <= 0.0f) rawDt = 0.016f;

        // 3. 处理输入
        Input::GetInstance().Update(rawDt);

        // 4. 处理帧
        game.ProcessFrame(rawDt);

        // 5. 帧率限制 (60 FPS)
        QueryPerformanceCounter(&now);
        float elapsed = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                      / freq.QuadPart;
        if (elapsed < TARGET_FRAME_TIME) {
            float waitTime = TARGET_FRAME_TIME - elapsed;
            DWORD sleepMs = static_cast<DWORD>(waitTime * 1000.0f);
            if (sleepMs > 1) {
                Sleep(sleepMs - 1);
            }
            do {
                QueryPerformanceCounter(&now);
                elapsed = static_cast<float>(now.QuadPart - frameStart.QuadPart)
                        / freq.QuadPart;
            } while (elapsed < TARGET_FRAME_TIME);
        }

        Input::GetInstance().EndFrame();
    }

    // ---- 清理 ----
    // 保存最高分
    std::string hsStr = std::to_string(game.GetHighScore());
    WriteProfileStringA("NeonStriker", "HighScore", hsStr.c_str());

    game.Shutdown();
    Renderer::GetInstance().Shutdown();
    EndBatchDraw();
    closegraph();

    return 0;
}

// ============================================================
// 编译说明
// ============================================================
// EasyX 库安装: https://easyx.cn/
//
// MinGW-w64 + EasyX for MinGW:
//   powershell -ExecutionPolicy Bypass -File .\build.ps1
//   or:
//   g++ -std=c++11 -finput-charset=UTF-8 -fexec-charset=GBK main.cpp *.cpp
//       -leasyx -lgdi32 -lole32 -lwinmm -o output\NeonStriker.exe
// ============================================================
