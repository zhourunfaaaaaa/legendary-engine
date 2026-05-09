// ============================================================
// WeaponTypes.cpp - 10 种武器多态实现
// 阶段 1/3：手枪、突击步枪、霰弹枪
// ============================================================

#include "../include/Weapon/WeaponTypes.h"
#include "../include/System/EntityManager.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>

// ============================================================
// 1. 突击步枪 (AssaultRifle)
//    MP: 1  射速: 8/s  伤害: 2  轻微随机散射 ±3°
// ============================================================
AssaultRifle::AssaultRifle()
    : m_accuracyOffset(3.0f) {      // 最大偏移角度 3°
    m_mpCost         = 1;  // 游侠武器低耗蓝
    m_fireRate       = 8.0f;
    m_baseDamage     = 2;
    m_spreadAngle    = 3.0f;
    m_projectileCount = 1;
    m_price          = 60;
}

void AssaultRifle::Fire(const Vector2& position, const Vector2& direction,
                        BulletFaction faction, EntityManager& entityMgr) {
    // 在散射范围内随机偏移
    float angleOffset = RandomFloat(-m_spreadAngle, m_spreadAngle);
    float baseAngle = std::atan2(direction.y, direction.x);
    float finalAngle = baseAngle + DegToRad(angleOffset);

    Vector2 scatteredDir(std::cos(finalAngle), std::sin(finalAngle));

    Bullet* bullet = CreateBullet(position, scatteredDir, faction, entityMgr);
    bullet->SetSpeed(600.0f);
    bullet->SetLifetime(2.5f);
    bullet->SetColor(0xFFAA22);     // 橙色
}

// ============================================================
// 3. 霰弹枪 (Shotgun)
//    MP: 3  射速: 1.5/s  伤害: 2  5 发扇形散布 ±15°
// ============================================================
Shotgun::Shotgun() {
    m_mpCost         = 2;  // 骑士武器中等耗蓝
    m_fireRate       = 1.5f;
    m_baseDamage     = 2;
    m_spreadAngle    = 30.0f;       // 总扇形角度 30°（±15°）
    m_projectileCount = 5;
    m_price          = 80;
}

void Shotgun::Fire(const Vector2& position, const Vector2& direction,
                   BulletFaction faction, EntityManager& entityMgr) {
    float baseAngle = std::atan2(direction.y, direction.x);

    int count = m_projectileCount;
    float totalSpread = DegToRad(m_spreadAngle);

    // 5 发子弹在扇形范围内均匀分布
    for (int i = 0; i < count; ++i) {
        // 从 -spread/2 到 +spread/2 均匀分布
        float t = (count > 1) ? (float)i / (count - 1) : 0.5f;
        float offsetAngle = -totalSpread * 0.5f + totalSpread * t;

        float finalAngle = baseAngle + offsetAngle;
        Vector2 bulletDir(std::cos(finalAngle), std::sin(finalAngle));

        Bullet* bullet = CreateBullet(position, bulletDir, faction, entityMgr);
        bullet->SetSpeed(450.0f);
        bullet->SetLifetime(1.8f);   // 霰弹射程较短
        bullet->SetColor(0xFFCC33);  // 金色
    }
}

void Shotgun::OnUpgrade() {
    // 散弹增加天赋：子弹数 5 -> 7
    if (m_level >= 2) {
        m_projectileCount = 7;
    }
}

// ============================================================
// 4. 狙击枪 (SniperRifle) - 穿透、高速、慢射速
// ============================================================
SniperRifle::SniperRifle() {
    m_mpCost         = 4;
    m_fireRate       = 1.0f;
    m_baseDamage     = 12;
    m_spreadAngle    = 0.0f;
    m_projectileCount = 1;
    m_price          = 120;
}

void SniperRifle::Fire(const Vector2& position, const Vector2& direction,
                       BulletFaction faction, EntityManager& entityMgr) {
    Bullet* bullet = CreateBullet(position, direction, faction, entityMgr);
    bullet->SetSpeed(2000.0f);
    bullet->SetLifetime(1.2f);
    bullet->SetPiercing(true);
    bullet->SetMaxPierceCount(3);
    bullet->SetColor(0xFF44FF);
    bullet->SetCollisionSize(12.0f, 12.0f);  // 增大碰撞盒防止高速穿透
}

// ============================================================
// 5. 火箭筒 (RocketLauncher) - 慢速爆炸弹
// ============================================================
RocketLauncher::RocketLauncher()
    : m_explosionRadius(80.0f), m_rocketSpeed(300.0f) {
    m_mpCost         = 5;
    m_fireRate       = 1.0f;
    m_baseDamage     = 15;
    m_spreadAngle    = 0.0f;
    m_projectileCount = 1;
    m_price          = 150;
}

void RocketLauncher::Fire(const Vector2& position, const Vector2& direction,
                          BulletFaction faction, EntityManager& entityMgr) {
    Bullet* rocket = CreateBullet(position, direction, faction, entityMgr);
    rocket->SetSpeed(m_rocketSpeed);
    rocket->SetLifetime(3.0f);
    rocket->SetExplosive(true);
    rocket->SetExplosionRadius(m_explosionRadius);
    rocket->SetColor(0xFF4400);
}

// ============================================================
// 7. 火焰机枪 (FlameThrower) - 短程密集火苗
// ============================================================
FlameThrower::FlameThrower()
    : m_flameRange(200.0f), m_burnChance(0.3f), m_burnDuration(3.0f)
    , m_flameCountPerShot(5) {
    m_mpCost         = 2;
    m_fireRate       = 6.0f;
    m_baseDamage     = 1;
    m_spreadAngle    = 15.0f;
    m_projectileCount = m_flameCountPerShot;
    m_price          = 100;
}

void FlameThrower::Fire(const Vector2& position, const Vector2& direction,
                        BulletFaction faction, EntityManager& entityMgr) {
    for (int i = 0; i < m_flameCountPerShot; ++i) {
        float angleOffset = RandomFloat(-m_spreadAngle, m_spreadAngle);
        float baseAngle = std::atan2(direction.y, direction.x);
        float finalAngle = baseAngle + DegToRad(angleOffset);
        Vector2 dir(std::cos(finalAngle), std::sin(finalAngle));
        Bullet* flame = CreateBullet(position, dir, faction, entityMgr);
        flame->SetSpeed(RandomFloat(250.0f, 400.0f));
        flame->SetLifetime(m_flameRange / 400.0f);
        flame->SetCausesBurning(true);
        flame->SetBurnChance(m_burnChance);
        flame->SetBurnDuration(m_burnDuration);
        flame->SetColor(0xFF6622);
    }
}

// ============================================================
// 8. 魔法法杖 (MagicStaff) - 追踪魔法弹
// ============================================================
MagicStaff::MagicStaff()
    : m_homingStrength(3.0f), m_magicBallSpeed(250.0f) {
    m_mpCost         = 1;  // 法师初始武器低耗蓝
    m_fireRate       = 2.0f;
    m_baseDamage     = 4;
    m_spreadAngle    = 0.0f;
    m_projectileCount = 3;
    m_price          = 130;
}

void MagicStaff::Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) {
    for (int i = 0; i < m_projectileCount; ++i) {
        float offset = (i == 0) ? -0.15f : 0.15f;
        float baseAngle = std::atan2(direction.y, direction.x);
        Vector2 dir(std::cos(baseAngle + offset), std::sin(baseAngle + offset));
        Bullet* ball = CreateBullet(position, dir, faction, entityMgr);
        ball->SetSpeed(m_magicBallSpeed);
        ball->SetLifetime(3.0f);
        ball->SetHoming(true);
        ball->SetHomingStrength(m_homingStrength);
        ball->SetColor(0xCC44FF);
    }
}

void MagicStaff::OnUpgrade() {
    if (m_level >= 2) m_projectileCount = 3;
}

// ============================================================
// 9. 反弹十字弩 (ReboundCrossbow) - 墙壁反弹
// ============================================================
ReboundCrossbow::ReboundCrossbow()
    : m_maxBounces(3), m_boltSpeed(600.0f) {
    m_mpCost         = 2;
    m_fireRate       = 3.0f;
    m_baseDamage     = 6;
    m_spreadAngle    = 0.0f;
    m_projectileCount = 1;
    m_price          = 110;
}

void ReboundCrossbow::Fire(const Vector2& position, const Vector2& direction,
                           BulletFaction faction, EntityManager& entityMgr) {
    Bullet* bolt = CreateBullet(position, direction, faction, entityMgr);
    bolt->SetSpeed(m_boltSpeed);
    bolt->SetLifetime(4.0f);
    bolt->SetBouncing(true);
    bolt->SetMaxBounces(m_maxBounces);
    bolt->SetColor(0x88AAFF);
}

// ============================================================
// 10. 吸血鬼法典 (VampireCodex) - 吸血蝙蝠
// ============================================================
VampireCodex::VampireCodex()
    : m_lifeStealChance(0.2f), m_lifeStealAmount(1), m_batSpeed(350.0f) {
    m_mpCost         = 4;
    m_fireRate       = 2.5f;
    m_baseDamage     = 4;
    m_spreadAngle    = 8.0f;
    m_projectileCount = 3;
    m_price          = 140;
}

void VampireCodex::Fire(const Vector2& position, const Vector2& direction,
                        BulletFaction faction, EntityManager& entityMgr) {
    for (int i = 0; i < m_projectileCount; ++i) {
        float t = m_projectileCount > 1 ? (float)i / (m_projectileCount - 1) : 0.5f;
        float offsetAngle = (t - 0.5f) * DegToRad(m_spreadAngle * 2);
        float baseAngle = std::atan2(direction.y, direction.x);
        Vector2 dir(std::cos(baseAngle + offsetAngle), std::sin(baseAngle + offsetAngle));
        Bullet* bat = CreateBullet(position, dir, faction, entityMgr);
        bat->SetSpeed(m_batSpeed);
        bat->SetLifetime(2.5f);
        bat->SetLifeSteal(true);
        bat->SetLifeStealChance(m_lifeStealChance);
        bat->SetColor(0x6600CC);
    }
}
