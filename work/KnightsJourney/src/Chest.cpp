// ============================================================
// Chest.cpp - 宝箱实现
// ============================================================

#include "../include/Entity/Chest.h"
#include "../include/Entity/Player.h"
#include "../include/Core/GameManager.h"
#include "../include/Graphics/VisualEffects.h"
#include "../include/Weapon/WeaponTypes.h"
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

    VisualFX::DrawPixelShadow(cx, cy + 18, 24, 10);

    if (m_isMimic && !m_isOpened) {
        // 宝箱怪伪装成宝箱
        VisualFX::DrawPixelRect(cx - 24, cy - 15, cx + 24, cy + 17,
                                RGB(184, 111, 42), RGB(29, 22, 18), 5);
        setfillcolor(RGB(222, 158, 52));
        solidrectangle(cx - 18, cy - 6, cx + 18, cy + 10);
        setfillcolor(RGB(255, 40, 40));
        solidrectangle(cx - 10, cy - 2, cx - 4, cy + 4);
        solidrectangle(cx + 4, cy - 2, cx + 10, cy + 4);
    } else if (m_isOpened) {
        VisualFX::DrawPixelRect(cx - 24, cy - 5, cx + 24, cy + 18,
                                RGB(96, 58, 24), RGB(30, 22, 15), 5);
        VisualFX::DrawPixelRect(cx - 22, cy - 19, cx + 22, cy - 6,
                                RGB(142, 87, 34), RGB(30, 22, 15), 4);
        // 内容物标识
        if (m_contentType == DropType::WEAPON_DROP) {
            int tier = GetWeaponTier(m_containedWeapon);
            COLORREF gem = tier == 3 ? RGB(255, 220, 86) :
                           tier == 2 ? RGB(120, 220, 255) :
                                       RGB(201, 133, 245);
            VisualFX::DrawPixelDiamond(cx, cy + 5, 8, gem, RGB(34, 20, 45));
        }
    } else {
        // 关闭的宝箱
        VisualFX::DrawPixelRect(cx - 24, cy - 15, cx + 24, cy + 17,
                                RGB(184, 111, 42), RGB(29, 22, 18), 5);
        setfillcolor(RGB(222, 158, 52));
        solidrectangle(cx - 18, cy - 6, cx + 18, cy + 10);
        setfillcolor(RGB(122, 74, 32));
        solidrectangle(cx - 22, cy - 14, cx + 22, cy - 6);
        // 锁扣
        VisualFX::DrawPixelRect(cx - 7, cy - 3, cx + 7, cy + 9,
                                RGB(238, 195, 63), RGB(60, 39, 12), 3);

        // 靠近时显示提示
        Player* player = GameManager::GetInstance().GetPlayer();
        if (player && !player->IsDead()) {
            float dist = Vector2::Distance(m_position, player->GetPosition());
            if (dist < 50.0f) {
                setbkmode(TRANSPARENT);
                settextcolor(RGB(255, 255, 200));
                settextstyle(12, 0, _T("Microsoft YaHei UI"));
                RECT rc = {cx - 50, cy - 40, cx + 50, cy - 20};
                drawtext(_T("按 E 打开"), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
