// ============================================================
// DropItem.cpp - 掉落物（金币/血瓶/蓝瓶/武器）
// ============================================================

#include "../include/Entity/DropItem.h"
#include "../include/Entity/Player.h"
#include "../include/Core/GameManager.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>

DropItem::DropItem()
    : m_dropType(DropType::GOLD)
    , m_value(1)
    , m_weaponType(WeaponType::ASSAULT_RIFLE)
    , m_magnetized(false)
    , m_magnetSpeed(420.0f)
    , m_magnetRange(100.0f)
    , m_aliveTime(0.0f)
    , m_maxAliveTime(15.0f)         // 15 秒后消失
    , m_bounceTimer(0.0f)
    , m_bounceOffset(0.0f) {

    SetRenderLayer(2);              // 底层渲染
    SetCollisionSize(8.0f, 8.0f);
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
            // 金色方块
            setfillcolor(RGB(80, 50, 10));
            solidrectangle(cx - halfSize - 1, cy - halfSize - 1,
                           cx + halfSize + 1, cy + halfSize + 1);
            setfillcolor(RGB(255, 215, 0));
            solidrectangle(cx - halfSize, cy - halfSize,
                           cx + halfSize, cy + halfSize);
            // 高光
            setfillcolor(RGB(255, 240, 120));
            solidrectangle(cx - 2, cy - 2, cx + 2, cy + 2);
            break;
        }
        case DropType::HP_POTION: {
            // 红色方块 + 十字
            setfillcolor(RGB(180, 30, 30));
            solidrectangle(cx - halfSize, cy - halfSize,
                           cx + halfSize, cy + halfSize);
            setfillcolor(RGB(255, 255, 255));
            solidrectangle(cx - 1, cy - 4, cx + 1, cy + 4);
            solidrectangle(cx - 4, cy - 1, cx + 4, cy + 1);
            break;
        }
        case DropType::MP_POTION: {
            // 蓝色方块 + 圆点
            setfillcolor(RGB(30, 60, 180));
            solidrectangle(cx - halfSize, cy - halfSize,
                           cx + halfSize, cy + halfSize);
            setfillcolor(RGB(200, 220, 255));
            solidcircle(cx, cy, 3);
            break;
        }
        case DropType::WEAPON_DROP: {
            // 紫色方块
            setfillcolor(RGB(100, 20, 160));
            solidrectangle(cx - halfSize, cy - halfSize,
                           cx + halfSize, cy + halfSize);
            setfillcolor(RGB(220, 160, 255));
            solidrectangle(cx - 3, cy - 3, cx + 3, cy + 3);
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
            player->Heal(healAmount);
            break;
        }
        case DropType::MP_POTION: {
            int mpAmount = m_value;
            if (mpAmount <= 0) mpAmount = 30;
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
