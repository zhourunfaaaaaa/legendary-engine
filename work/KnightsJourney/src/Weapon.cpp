// ============================================================
// Weapon.cpp - 武器基类实现
// ============================================================

#include "../include/Weapon/Weapon.h"
#include "../include/Entity/Player.h"
#include "../include/System/EntityManager.h"
#include <cstdio>

// ============================================================
// 构造
// ============================================================
Weapon::Weapon()
    : m_mpCost(0)
    , m_fireRate(3.0f)
    , m_cooldownTimer(0.0f)
    , m_fireRateMultiplier(1.0f)
    , m_baseDamage(2)
    , m_spreadAngle(0.0f)
    , m_critChance(0.0f)
    , m_projectileCount(1)
    , m_level(1)
    , m_price(0) {
}

// ============================================================
// 冷却更新
// ============================================================
void Weapon::UpdateCooldown(float deltaTime) {
    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= deltaTime * m_fireRateMultiplier;
        if (m_cooldownTimer < 0.0f) {
            m_cooldownTimer = 0.0f;
        }
    }
}

// ============================================================
// 是否可以开火（检查冷却 + MP）
// ============================================================
bool Weapon::CanFire(Player* player) const {
    if (m_cooldownTimer > 0.0f) return false;
    if (!player) return true;  // 无玩家上下文时允许（NPC 使用场景）
    return player->GetMP() >= m_mpCost;
}

// ============================================================
// 子弹工厂方法 - 创建一颗基础子弹并设置通用属性
// ============================================================
Bullet* Weapon::CreateBullet(const Vector2& position, const Vector2& direction,
                              BulletFaction faction, EntityManager& entityMgr) {
    Bullet* bullet = entityMgr.SpawnEntity<Bullet>();

    // 基础属性
    bullet->SetPosition(position);
    bullet->SetDirection(direction);
    bullet->SetFaction(faction);
    bullet->SetDamage(m_baseDamage);
    bullet->SetSourceWeapon(GetType());

    // 暴击判定
    if (m_critChance > 0.0f) {
        float roll = RandomFloat(0.0f, 1.0f);
        if (roll <= m_critChance) {
            bullet->SetCritical(true);
        }
    }

    // 更新冷却（单发间隔 = 1.0 / fireRate）
    m_cooldownTimer = 1.0f / m_fireRate;

    return bullet;
}
