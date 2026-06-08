// ============================================================
// DropItem.cpp - 掉落物（金币/血瓶/蓝瓶/武器）
// ============================================================

#include "../include/Entity/DropItem.h"
#include "../include/Entity/Player.h"
#include "../include/Core/GameManager.h"
#include "../include/System/BuffManager.h"
#include "../include/Graphics/VisualEffects.h"
#include "../include/Weapon/WeaponTypes.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>

DropItem::DropItem()
    : m_dropType(DropType::GOLD)
    , m_value(1)
    , m_weaponType(WeaponType::ASSAULT_RIFLE)
    , m_magnetized(false)
    , m_magnetSpeed(620.0f)
    , m_magnetRange(180.0f)
    , m_aliveTime(0.0f)
    , m_maxAliveTime(15.0f)         // 15 秒后消失
    , m_bounceTimer(0.0f)
    , m_bounceOffset(0.0f) {

    SetRenderLayer(2);              // 底层渲染
    SetCollisionSize(8.0f, 8.0f);
    m_bounceTimer = (float)((GetID() * 7) % 23) * 0.19f;
}

void DropItem::Update(float deltaTime) {
    m_aliveTime += deltaTime;

    // 超时消失
    if (m_aliveTime >= m_maxAliveTime) {
        MarkForDeletion();
        return;
    }

    // 弹跳动画
    m_bounceTimer += deltaTime * 4.0f;
    m_bounceOffset = std::sin(m_bounceTimer) * 3.0f;

    // 磁吸效果：检测玩家距离
    Player* player = GameManager::GetInstance().GetPlayer();
    if (player && !player->IsDead()) {
        float dist = Vector2::Distance(m_position, player->GetPosition());
        if (dist < m_magnetRange) {
            m_magnetized = true;
        }
        if (m_magnetized) {
            Vector2 dir = (player->GetPosition() - m_position).Normalized();
            m_position += dir * m_magnetSpeed * deltaTime;
        }
    }

    SyncAABBToPosition();
}

void DropItem::Render() {
    int cx = static_cast<int>(m_position.x);
    int cy = static_cast<int>(m_position.y) + static_cast<int>(m_bounceOffset);

    int halfSize = 6;

    switch (m_dropType) {
        case DropType::GOLD: {
            VisualFX::DrawPixelShadow(cx, cy + 10, 12, 4);
            VisualFX::DrawPixelDiamond(cx, cy, 12, RGB(246, 196, 47), RGB(73, 51, 16));
            setfillcolor(RGB(255, 237, 117));
            solidrectangle(cx - 3, cy - 5, cx + 5, cy - 1);
            break;
        }
        case DropType::HP_POTION: {
            VisualFX::DrawPixelShadow(cx, cy + 10, 12, 4);
            VisualFX::DrawPixelRect(cx - 9, cy - 12, cx + 9, cy + 11,
                                    RGB(205, 56, 67), RGB(44, 19, 22), 3);
            setfillcolor(RGB(237, 224, 204));
            solidrectangle(cx - 5, cy - 17, cx + 5, cy - 10);
            setfillcolor(RGB(255, 255, 255));
            solidrectangle(cx - 1, cy - 4, cx + 1, cy + 4);
            solidrectangle(cx - 4, cy - 1, cx + 4, cy + 1);
            break;
        }
        case DropType::MP_POTION: {
            VisualFX::DrawPixelShadow(cx, cy + 10, 12, 4);
            VisualFX::DrawPixelRect(cx - 9, cy - 12, cx + 9, cy + 11,
                                    RGB(53, 102, 210), RGB(18, 28, 58), 3);
            setfillcolor(RGB(220, 232, 255));
            solidrectangle(cx - 5, cy - 17, cx + 5, cy - 10);
            setfillcolor(RGB(200, 220, 255));
            solidrectangle(cx - 3, cy - 2, cx + 3, cy + 4);
            break;
        }
        case DropType::WEAPON_DROP: {
            int tier = GetWeaponTier(m_weaponType);
            COLORREF fill = tier == 3 ? RGB(179, 122, 46) :
                            tier == 2 ? RGB(65, 111, 176) :
                                        RGB(123, 72, 185);
            COLORREF shine = tier == 3 ? RGB(255, 226, 112) :
                             tier == 2 ? RGB(168, 226, 255) :
                                         RGB(214, 168, 255);
            VisualFX::DrawPixelShadow(cx, cy + 12, 16, 5);
            VisualFX::DrawPixelRect(cx - 15, cy - 12, cx + 15, cy + 12,
                                    fill, RGB(33, 21, 52), 4);
            setfillcolor(shine);
            solidrectangle(cx - 7, cy - 3, cx + 8, cy + 4);
            setfillcolor(RGB(241, 226, 255));
            solidrectangle(cx + 5, cy - 8, cx + 12, cy - 3);
            break;
        }
    }
}

void DropItem::OnPickup(Player* player) {
    if (!player) return;

    switch (m_dropType) {
        case DropType::GOLD: {
            int amount = m_value;
            GameManager::GetInstance().AddGold(amount);
            break;
        }
        case DropType::HP_POTION: {
            int healAmount = m_value;  // value 为回复量
            if (healAmount <= 0) healAmount = 2;
            if (GameManager::GetInstance().GetBuffManager().HasPotionMastery()) healAmount += 1;
            player->Heal(healAmount);
            break;
        }
        case DropType::MP_POTION: {
            int mpAmount = m_value;
            if (mpAmount <= 0) mpAmount = 30;
            if (GameManager::GetInstance().GetBuffManager().HasPotionMastery()) mpAmount += 25;
            player->SetMP(player->GetMP() + mpAmount);
            break;
        }
        case DropType::WEAPON_DROP: {
            if (!player->EquipWeapon(m_weaponType)) {
                // 武器槽满，触发选择
                GameManager::GetInstance().SetState(GameState::WEAPON_SELECT);
            }
            break;
        }
    }

    MarkForDeletion();
}
