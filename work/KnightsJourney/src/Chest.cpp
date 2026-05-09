// ============================================================
// Chest.cpp - 宝箱实现
// ============================================================

#include "../include/Entity/Chest.h"
#include "../include/Entity/Player.h"
#include "../include/Core/GameManager.h"
#include <graphics.h>
#include <cstdio>

Chest::Chest()
    : m_isOpened(false), m_isMimic(false), m_mimicChance(0.05f)
    , m_contentType(DropType::GOLD), m_containedWeapon(WeaponType::ASSAULT_RIFLE)
    , m_contentValue(10), m_sourceRoom(RoomType::REWARD)
    , m_openAnimationTimer(0.0f), m_isAnimating(false) {
    SetRenderLayer(4);
    SetCollisionSize(16.0f, 12.0f);
}

void Chest::Update(float deltaTime) {
    if (m_isAnimating) {
        m_openAnimationTimer -= deltaTime;
        if (m_openAnimationTimer <= 0.0f) m_isAnimating = false;
    }
    SyncAABBToPosition();
}

void Chest::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;

    if (m_isMimic && !m_isOpened) {
        // 宝箱怪伪装成宝箱
        setfillcolor(RGB(120, 80, 20));
        solidrectangle(cx - 14, cy - 10, cx + 14, cy + 10);
        setfillcolor(RGB(200, 160, 40));
        solidrectangle(cx - 12, cy - 8, cx + 12, cy + 8);
        setfillcolor(RGB(255, 40, 40));
        solidcircle(cx - 4, cy, 3);
        solidcircle(cx + 4, cy, 3);
    } else if (m_isOpened) {
        setfillcolor(RGB(60, 40, 10));
        solidrectangle(cx - 14, cy - 4, cx + 14, cy + 12);
        setfillcolor(RGB(140, 100, 30));
        solidrectangle(cx - 12, cy - 2, cx + 12, cy + 10);
        // 内容物标识
        if (m_contentType == DropType::WEAPON_DROP) {
            setfillcolor(RGB(220, 160, 255));
            solidcircle(cx, cy + 4, 4);
        }
    } else {
        // 关闭的宝箱
        setfillcolor(RGB(100, 60, 15));
        solidrectangle(cx - 14, cy - 10, cx + 14, cy + 10);
        setfillcolor(RGB(180, 140, 30));
        solidrectangle(cx - 12, cy - 8, cx + 12, cy + 8);
        // 锁扣
        setfillcolor(RGB(255, 215, 0));
        solidcircle(cx, cy, 4);

        // 靠近时显示提示
        Player* player = GameManager::GetInstance().GetPlayer();
        if (player && !player->IsDead()) {
            float dist = Vector2::Distance(m_position, player->GetPosition());
            if (dist < 50.0f) {
                setbkmode(TRANSPARENT);
                settextcolor(RGB(255, 255, 200));
                settextstyle(12, 0, _T("Consolas"));
                RECT rc = {cx - 50, cy - 40, cx + 50, cy - 20};
                drawtext(_T("Press E to open"), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
}

bool Chest::Open() {
    if (m_isOpened) return false;

    m_isOpened = true;
    m_isAnimating = true;
    m_openAnimationTimer = 0.5f;

    if (m_isMimic) {
        printf("[Chest] It's a mimic!\n");
    }
    return m_isMimic;
}

void Chest::SetAsWeaponDrop(WeaponType wt) { m_contentType = DropType::WEAPON_DROP; m_containedWeapon = wt; }
void Chest::SetAsHPPotion(int amount)      { m_contentType = DropType::HP_POTION; m_contentValue = amount; }
void Chest::SetAsMPPotion(int amount)      { m_contentType = DropType::MP_POTION; m_contentValue = amount; }
void Chest::SetAsGold(int amount)          { m_contentType = DropType::GOLD; m_contentValue = amount; }
void Chest::SetAsMimic()                   { m_isMimic = true; }
