#pragma once

#include <windows.h>
#include "Common.h"

// ============================================================
// Input - 键盘+鼠标输入管理（单例）
// 使用 GetAsyncKeyState 轮询，无需键盘钩子
// ============================================================
class Input {
public:
    static Input& GetInstance() {
        static Input instance;
        return instance;
    }

    void Init();
    void Update(float dt);
    void EndFrame();

    bool IsDown(int vkCode) const;
    bool WasPressed(int vkCode) const;

    Vector2 GetKeyboardDir() const;
    Vector2 GetDragDelta();

    bool IsBulletTime() const;

    int  GetMouseX() const { return m_mouseX; }
    int  GetMouseY() const { return m_mouseY; }
    bool IsMouseDown() const { return m_mouseDown; }
    bool WasMousePressed() const { return m_mouseJustPressed; }

    void ResetOnEnterGame();
    void ResetInputState();

private:
    Input() = default;

    bool m_keys[256];
    bool m_justPressed[256];
    bool m_prevKeys[256];

    int  m_mouseX, m_mouseY;
    int  m_prevMouseX, m_prevMouseY;
    bool m_mouseDown;
    bool m_mouseJustPressed;

    bool  m_dragging;
    float m_dragDx, m_dragDy;
    int   m_lastPointerX, m_lastPointerY;

    float m_inputLockTimer;
};
