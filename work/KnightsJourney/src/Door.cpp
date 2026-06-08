// ============================================================
// Door.cpp - 房间门实现
// ============================================================

#include "../include/Entity/Door.h"
#include "../include/Graphics/VisualEffects.h"
#include <graphics.h>

Door::Door()
    : m_isOpen(false), m_isLocked(true), m_direction(DoorDirection::NORTH)
    , m_openTimer(0.0f) {
    SetRenderLayer(1);
    SetCollisionSize(24.0f, 24.0f);
}

void Door::Update(float deltaTime) {
    if (m_openTimer > 0.0f) m_openTimer -= deltaTime;
    SyncAABBToPosition();
}

void Door::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;

    COLORREF frameColor = m_isLocked ? RGB(164, 62, 57) : (m_isOpen ? RGB(74, 178, 104) : RGB(205, 166, 72));
    COLORREF innerColor = m_isOpen ? RGB(26, 81, 49) : RGB(87, 55, 32);

    VisualFX::DrawPixelShadow(cx, cy + 26, 24, 10);
    VisualFX::DrawPixelRect(cx - 24, cy - 28, cx + 24, cy + 28,
                            frameColor, RGB(10, 12, 14), 5);
    VisualFX::DrawPixelRect(cx - 14, cy - 18, cx + 14, cy + 20,
                            innerColor, RGB(30, 24, 20), 4);

    if (m_isLocked) {
        VisualFX::DrawPixelRect(cx - 7, cy - 2, cx + 7, cy + 12,
                                RGB(211, 176, 65), RGB(42, 30, 12), 3);
    } else if (m_isOpen) {
        setfillcolor(RGB(123, 229, 137));
        solidrectangle(cx - 5, cy - 19, cx + 5, cy - 12);
    }

    // 方向箭头
    setfillcolor(m_isLocked ? RGB(255, 178, 178) : RGB(255, 245, 180));
    switch (m_direction) {
        case DoorDirection::NORTH: solidrectangle(cx - 4, cy - 12, cx + 4, cy - 2); solidcircle(cx, cy - 13, 3); break;
        case DoorDirection::SOUTH: solidrectangle(cx - 4, cy + 2, cx + 4, cy + 12); solidcircle(cx, cy + 13, 3); break;
        case DoorDirection::EAST:  solidrectangle(cx + 2, cy - 4, cx + 12, cy + 4); solidcircle(cx + 13, cy, 3); break;
        case DoorDirection::WEST:  solidrectangle(cx - 12, cy - 4, cx - 2, cy + 4); solidcircle(cx - 13, cy, 3); break;
        default: break;
    }
}

void Door::Unlock() { m_isLocked = false; }
void Door::Open()  { if (!m_isLocked) { m_isOpen = true; m_openTimer = 0.3f; } }
void Door::Close() { m_isOpen = false; }

bool Door::IsPlayerNearby(const Vector2& playerPos) const {
    float dx = m_position.x - playerPos.x;
    float dy = m_position.y - playerPos.y;
    return (dx * dx + dy * dy) < (DOOR_INTERACT_RANGE * DOOR_INTERACT_RANGE);
}
