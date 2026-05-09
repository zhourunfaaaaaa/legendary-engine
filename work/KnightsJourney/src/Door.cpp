// ============================================================
// Door.cpp - 房间门实现
// ============================================================

#include "../include/Entity/Door.h"
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

    COLORREF frameColor = m_isLocked ? RGB(120, 30, 30) : (m_isOpen ? RGB(60, 200, 60) : RGB(180, 180, 80));
    setfillcolor(frameColor);
    solidrectangle(cx - 14, cy - 20, cx + 14, cy + 20);

    setfillcolor(m_isOpen ? RGB(20, 50, 20) : RGB(50, 30, 10));
    solidrectangle(cx - 10, cy - 16, cx + 10, cy + 16);

    // 方向箭头
    setfillcolor(RGB(255, 255, 200));
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
