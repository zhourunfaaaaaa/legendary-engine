#include "Input.h"
#include <windows.h>
#include <graphics.h>

void Input::Init() {
    ResetInputState();
}

// 检查按键是否在当前帧刚按下
static bool CheckJustPressed(int vk) {
    static bool prev[256] = {false};
    bool cur = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool result = cur && !prev[vk];
    prev[vk] = cur;
    return result;
}

// 检查按键是否按住
static bool CheckDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void Input::Update(float dt) {
    if (m_inputLockTimer > 0.0f) m_inputLockTimer -= dt;

    // ── 键盘轮询 ──
    for (int i = 0; i < 256; i++) {
        bool cur = CheckDown(i);
        m_justPressed[i] = cur && !m_prevKeys[i];
        m_keys[i] = cur;
    }

    // ── 鼠标轮询 ──
    m_prevMouseX = m_mouseX;
    m_prevMouseY = m_mouseY;
    POINT pt;
    GetCursorPos(&pt);
    HWND hwnd = GetHWnd();
    if (hwnd) ScreenToClient(hwnd, &pt);
    m_mouseX = pt.x;
    m_mouseY = pt.y;

    bool nowDown = CheckDown(VK_LBUTTON);
    m_mouseJustPressed = nowDown && !m_mouseDown;
    m_mouseDown = nowDown;

    // ── 拖拽逻辑 ──
    if (m_mouseJustPressed && m_inputLockTimer <= 0.0f) {
        m_dragging = true;
        m_dragDx = 0.0f;
        m_dragDy = 0.0f;
        m_lastPointerX = m_mouseX;
        m_lastPointerY = m_mouseY;
    }
    if (!nowDown) {
        m_dragging = false;
        m_dragDx = 0.0f;
        m_dragDy = 0.0f;
    }

    if (m_dragging && m_inputLockTimer <= 0.0f) {
        float rawDx = (float)(m_mouseX - m_lastPointerX);
        float rawDy = (float)(m_mouseY - m_lastPointerY);
        m_dragDx += rawDx;
        m_dragDy += rawDy;
        m_lastPointerX = m_mouseX;
        m_lastPointerY = m_mouseY;
    }
}

void Input::EndFrame() {
    for (int i = 0; i < 256; i++) {
        m_prevKeys[i] = m_keys[i];
        m_justPressed[i] = false;
    }
    m_mouseJustPressed = false;
}

bool Input::IsDown(int vkCode) const {
    return m_keys[vkCode];
}

bool Input::WasPressed(int vkCode) const {
    return m_justPressed[vkCode];
}

Vector2 Input::GetKeyboardDir() const {
    float dx = 0.0f, dy = 0.0f;
    if (IsDown(VK_LEFT)  || IsDown('A')) dx -= 1.0f;
    if (IsDown(VK_RIGHT) || IsDown('D')) dx += 1.0f;
    if (IsDown(VK_UP)    || IsDown('W')) dy -= 1.0f;
    if (IsDown(VK_DOWN)  || IsDown('S')) dy += 1.0f;

    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 1.0f) { dx /= len; dy /= len; }
    return Vector2(dx, dy);
}

Vector2 Input::GetDragDelta() {
    if (!m_dragging || m_inputLockTimer > 0.0f) {
        m_dragDx = 0.0f;
        m_dragDy = 0.0f;
        return Vector2(0.0f, 0.0f);
    }

    constexpr float MAX_DRAG = 80.0f;
    float dx = ClampFloat(m_dragDx, -MAX_DRAG, MAX_DRAG);
    float dy = ClampFloat(m_dragDy, -MAX_DRAG, MAX_DRAG);
    m_dragDx = 0.0f;
    m_dragDy = 0.0f;
    return Vector2(dx, dy);
}

bool Input::IsBulletTime() const {
    return IsDown(VK_SPACE) || IsDown(VK_LSHIFT) || IsDown(VK_RSHIFT);
}

void Input::ResetOnEnterGame() {
    ResetInputState();
    m_inputLockTimer = INPUT_LOCK_TIME;
}

void Input::ResetInputState() {
    for (int i = 0; i < 256; i++) {
        m_keys[i] = false;
        m_justPressed[i] = false;
        m_prevKeys[i] = false;
    }
    m_dragging = false;
    m_dragDx = 0.0f;
    m_dragDy = 0.0f;
    m_mouseDown = false;
    m_mouseJustPressed = false;
    m_prevMouseX = m_mouseX;
    m_prevMouseY = m_mouseY;
}
