// ============================================================
// WeaponTypes.cpp - 10 种武器多态实现
// 阶段 1/3：手枪、突击步枪、霰弹枪
// ============================================================

#include "../include/Weapon/WeaponTypes.h"
#include "../include/System/EntityManager.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

namespace {

const WeaponSpec g_weaponSpecs[] = {
    { WeaponType::ASSAULT_RIFLE, "突击步枪", "步枪", "稳定高速连射，适合中距离压制", 1, 1, 8.0f, 2, 1, 3.0f, 760.0f, 2.5f, 0xFFAA22, 55, WeaponPattern::SINGLE, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::SHOTGUN, "霰弹枪", "霰弹", "扇形多弹，近距离爆发强", 1, 2, 1.5f, 3, 5, 30.0f, 450.0f, 1.8f, 0xFFCC33, 70, WeaponPattern::SPREAD, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::SNIPER_RIFLE, "狙击枪", "狙击", "瞬发穿透光线，单线高伤", 2, 4, 1.0f, 12, 1, 0.0f, 2000.0f, 1.2f, 0xFF44FF, 120, WeaponPattern::LASER, true, false, false, false, 3, 0, 0.0f, 0.0f },
    { WeaponType::ROCKET_LAUNCHER, "火箭筒", "火箭", "慢速重型榴弹，爆炸范围大", 2, 5, 0.9f, 14, 1, 0.0f, 285.0f, 4.0f, 0xFF4400, 140, WeaponPattern::EXPLOSIVE, false, true, false, false, 1, 0, 96.0f, 0.0f },
    { WeaponType::FLAME_THROWER, "火焰机枪", "火焰", "短程密集火苗，持续点燃", 1, 2, 6.0f, 1, 5, 16.0f, 360.0f, 0.55f, 0xFF6622, 80, WeaponPattern::FLAME, false, true, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::MAGIC_STAFF, "魔法法杖", "法杖", "发射追踪魔法弹", 1, 1, 2.0f, 3, 3, 18.0f, 275.0f, 3.2f, 0xCC44FF, 80, WeaponPattern::HOMING, false, false, false, false, 1, 0, 0.0f, 3.0f },
    { WeaponType::REBOUND_CROSSBOW, "反弹十字弩", "弩", "弩箭碰墙反弹，清角落很强", 1, 2, 3.0f, 6, 1, 0.0f, 650.0f, 4.0f, 0x88AAFF, 85, WeaponPattern::BOUNCE, false, false, false, false, 1, 3, 0.0f, 0.0f },
    { WeaponType::VAMPIRE_CODEX, "吸血魔典", "魔典", "蝙蝠弹命中有概率回血", 2, 4, 2.5f, 4, 3, 16.0f, 380.0f, 2.8f, 0x6600CC, 115, WeaponPattern::CODEX, false, false, false, true, 1, 0, 0.0f, 1.6f },
    { WeaponType::BURST_SMG, "三连冲锋枪", "冲锋", "三连点射，移动战很顺手", 1, 1, 4.2f, 2, 3, 8.0f, 720.0f, 2.1f, 0x77D6FF, 65, WeaponPattern::BURST, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::HAND_CANNON, "手炮", "手炮", "慢速重弹，击中很疼", 1, 2, 1.4f, 7, 1, 2.0f, 520.0f, 2.4f, 0xFFC06A, 75, WeaponPattern::SINGLE, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::FROST_PISTOL, "寒霜手枪", "冰枪", "低耗冰弹，附带减速", 1, 1, 3.2f, 3, 1, 5.0f, 620.0f, 2.4f, 0x88DDFF, 70, WeaponPattern::SINGLE, false, false, true, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::SPARK_CARBINE, "电弧卡宾", "电弧", "较快射速，弹道更亮更准", 1, 1, 5.4f, 3, 1, 2.0f, 780.0f, 2.2f, 0x66F2FF, 78, WeaponPattern::SINGLE, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::BONE_BOW, "骨弓", "骨弓", "中速穿刺箭，可穿过两个目标", 1, 2, 2.1f, 5, 1, 1.0f, 760.0f, 2.8f, 0xDDD6B0, 82, WeaponPattern::SINGLE, true, false, false, false, 2, 0, 0.0f, 0.0f },
    { WeaponType::TOXIC_SPRAYER, "毒雾喷射器", "毒喷", "宽散布弹幕，近身压制", 1, 2, 4.5f, 2, 4, 24.0f, 390.0f, 1.25f, 0x77CC55, 88, WeaponPattern::FLAME, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::CHAIN_BLASTER, "链式爆裂枪", "链爆", "双发爆裂弹，范围较小但稳定", 2, 3, 2.4f, 5, 2, 10.0f, 560.0f, 2.5f, 0xFF8844, 125, WeaponPattern::EXPLOSIVE, false, true, false, false, 1, 0, 52.0f, 0.0f },
    { WeaponType::ARCANE_ORB, "奥术球杖", "奥球", "慢速大奥术球，追踪并压走位", 2, 3, 1.55f, 7, 1, 0.0f, 210.0f, 5.0f, 0xB873FF, 128, WeaponPattern::HOMING, false, false, false, false, 1, 0, 0.0f, 4.8f },
    { WeaponType::CRYSTAL_LASER, "水晶激光枪", "激光", "连续光束段，穿透清线", 2, 3, 1.75f, 6, 1, 0.0f, 1800.0f, 0.7f, 0x77F7FF, 132, WeaponPattern::LASER, true, false, false, false, 4, 0, 0.0f, 0.0f },
    { WeaponType::THUNDER_RAIL, "雷轨炮", "雷轨", "粗雷束贯穿整条走廊", 2, 4, 1.0f, 13, 1, 0.0f, 2200.0f, 0.75f, 0x66AAFF, 145, WeaponPattern::LASER, true, false, false, false, 5, 0, 0.0f, 0.0f },
    { WeaponType::DRAGON_SHOTGUN, "龙息霰弹枪", "龙息", "燃烧霰弹，贴脸极强", 2, 3, 1.35f, 4, 6, 38.0f, 480.0f, 1.7f, 0xFF7733, 135, WeaponPattern::SPREAD, false, true, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::PLASMA_RIFLE, "等离子步枪", "电浆", "高稳定高射速，二阶通用强枪", 2, 2, 6.6f, 4, 1, 2.0f, 860.0f, 2.4f, 0x66FFCC, 140, WeaponPattern::SINGLE, false, false, false, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::VOID_LAUNCHER, "虚空发射器", "虚空", "缓慢虚空球，超大范围爆裂", 3, 6, 0.78f, 18, 1, 0.0f, 235.0f, 4.8f, 0x9955FF, 210, WeaponPattern::EXPLOSIVE, false, false, false, false, 1, 0, 150.0f, 0.0f },
    { WeaponType::STORM_STAFF, "风暴法杖", "风暴", "多枚强追踪闪电球", 3, 4, 2.0f, 6, 4, 26.0f, 360.0f, 3.4f, 0x78E8FF, 205, WeaponPattern::HOMING, false, false, false, false, 1, 0, 0.0f, 5.2f },
    { WeaponType::PHOENIX_CROSSBOW, "凤凰弩", "凤凰", "燃烧反弹弩箭，弹道压场", 3, 4, 2.8f, 8, 2, 10.0f, 760.0f, 4.5f, 0xFFAA44, 215, WeaponPattern::BOUNCE, false, true, false, false, 1, 4, 0.0f, 0.0f },
    { WeaponType::BLOOD_SCYTHE, "血月镰", "血镰", "吸血弧形弹，清群回血", 3, 5, 2.2f, 7, 5, 42.0f, 450.0f, 2.6f, 0xCC3355, 220, WeaponPattern::CODEX, false, false, false, true, 1, 0, 0.0f, 1.2f },
    { WeaponType::STAR_CANNON, "星陨炮", "星炮", "星形散射爆发，消耗较高", 3, 5, 1.4f, 8, 8, 360.0f, 620.0f, 2.6f, 0xFFE766, 230, WeaponPattern::ORBITAL, true, false, false, false, 2, 0, 0.0f, 0.0f },
    { WeaponType::GLACIER_MINIGUN, "冰川机枪", "冰机", "高速冰弹，减速覆盖很强", 3, 3, 9.2f, 3, 1, 4.0f, 880.0f, 2.5f, 0xA7F1FF, 225, WeaponPattern::SINGLE, false, false, true, false, 1, 0, 0.0f, 0.0f },
    { WeaponType::METEOR_ROD, "陨星权杖", "陨星", "慢速追踪陨石，落点爆炸", 3, 5, 1.35f, 11, 2, 18.0f, 255.0f, 4.6f, 0xFF6633, 235, WeaponPattern::HOMING, false, true, false, false, 1, 0, 92.0f, 4.0f },
    { WeaponType::SUNFIRE_SPEAR, "日炎长矛", "日矛", "燃烧光矛束，穿透点燃", 3, 4, 1.8f, 10, 1, 0.0f, 1450.0f, 1.6f, 0xFFD45A, 225, WeaponPattern::LASER, true, true, false, false, 5, 0, 0.0f, 0.0f },
    { WeaponType::SHADOW_CODEX, "暗影魔典", "暗典", "暗影蝠群，强吸血", 3, 5, 2.6f, 6, 6, 35.0f, 520.0f, 3.0f, 0x663399, 230, WeaponPattern::CODEX, false, false, false, true, 1, 0, 0.0f, 2.4f },
    { WeaponType::EMPEROR_RIFLE, "帝王步枪", "帝枪", "终阶步枪，极准高伤高射速", 3, 3, 7.6f, 6, 1, 1.0f, 980.0f, 2.8f, 0xFFE08A, 240, WeaponPattern::SINGLE, true, false, false, false, 2, 0, 0.0f, 0.0f }
};

int WeaponIndex(WeaponType type) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= static_cast<int>(sizeof(g_weaponSpecs) / sizeof(g_weaponSpecs[0]))) return 0;
    return idx;
}

} // namespace

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
    bullet->SetSpeed(760.0f);
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
    m_baseDamage     = 3;
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
    m_baseDamage     = 3;
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

const WeaponSpec& GetWeaponSpec(WeaponType type) {
    return g_weaponSpecs[WeaponIndex(type)];
}

const char* GetWeaponDisplayName(WeaponType type) {
    return GetWeaponSpec(type).name;
}

const char* GetWeaponShortName(WeaponType type) {
    return GetWeaponSpec(type).shortName;
}

const char* GetWeaponDescription(WeaponType type) {
    return GetWeaponSpec(type).description;
}

int GetWeaponTier(WeaponType type) {
    return GetWeaponSpec(type).tier;
}

int GetWeaponPrice(WeaponType type) {
    return GetWeaponSpec(type).price;
}

int GetWeaponCount() {
    return static_cast<int>(sizeof(g_weaponSpecs) / sizeof(g_weaponSpecs[0]));
}

WeaponType RollWeaponByTier(int worldLevel, bool shopRoll) {
    int t1 = 70, t2 = 25, t3 = 5;
    if (worldLevel >= 2) { t1 = 45; t2 = 40; t3 = 15; }
    if (worldLevel >= 3) { t1 = 25; t2 = 45; t3 = 30; }
    if (shopRoll) {
        t1 = (std::max)(10, t1 - 10);
        t2 += 5;
        t3 += 5;
    }

    int roll = RandomInt(1, t1 + t2 + t3);
    int wantedTier = roll <= t1 ? 1 : (roll <= t1 + t2 ? 2 : 3);
    std::vector<WeaponType> pool;
    for (int i = 0; i < GetWeaponCount(); ++i) {
        if (g_weaponSpecs[i].tier == wantedTier) {
            pool.push_back(g_weaponSpecs[i].type);
        }
    }
    if (pool.empty()) return WeaponType::ASSAULT_RIFLE;
    return pool[RandomInt(0, (int)pool.size() - 1)];
}

std::unique_ptr<Weapon> CreateWeaponByType(WeaponType type) {
    if (type == WeaponType::COUNT) type = WeaponType::ASSAULT_RIFLE;
    return std::unique_ptr<Weapon>(new GenericWeapon(type));
}

GenericWeapon::GenericWeapon(WeaponType type)
    : m_type(type) {
    const WeaponSpec& s = GetWeaponSpec(type);
    m_mpCost = s.mpCost;
    m_fireRate = s.fireRate;
    m_baseDamage = s.damage;
    m_spreadAngle = s.spreadAngle;
    m_projectileCount = s.projectileCount;
    m_price = s.price;
}

const char* GenericWeapon::GetName() const {
    return GetWeaponDisplayName(m_type);
}

const char* GenericWeapon::GetDescription() const {
    return GetWeaponDescription(m_type);
}

Bullet* GenericWeapon::SpawnSpecBullet(const Vector2& position, const Vector2& direction,
                                       BulletFaction faction, EntityManager& entityMgr,
                                       float speedScale, float lifetimeScale) {
    const WeaponSpec& s = GetWeaponSpec(m_type);
    Bullet* bullet = CreateBullet(position, direction, faction, entityMgr);
    bullet->SetSpeed(s.bulletSpeed * speedScale);
    bullet->SetLifetime(s.lifetime * lifetimeScale);
    bullet->SetColor(s.color);

    if (s.piercing) {
        bullet->SetPiercing(true);
        bullet->SetMaxPierceCount(s.maxPierce);
    }
    if (s.burning) {
        bullet->SetCausesBurning(true);
        bullet->SetBurnChance(s.tier >= 3 ? 0.55f : 0.35f);
        bullet->SetBurnDuration(s.tier >= 3 ? 3.0f : 2.0f);
    }
    if (s.slowing) {
        bullet->SetCausesSlow(true);
        bullet->SetSlowDuration(s.tier >= 3 ? 2.4f : 1.6f);
    }
    if (s.lifeSteal) {
        bullet->SetLifeSteal(true);
        bullet->SetLifeStealChance(s.tier >= 3 ? 0.28f : 0.18f);
    }
    if (s.maxBounces > 0) {
        bullet->SetBouncing(true);
        bullet->SetMaxBounces(s.maxBounces);
    }
    if (s.explosionRadius > 0.0f) {
        bullet->SetExplosive(true);
        bullet->SetExplosionRadius(s.explosionRadius);
        if (s.explosionRadius >= 80.0f) {
            bullet->SetCollisionSize(17.0f, 17.0f);
        }
    }
    if (s.homingStrength > 0.0f) {
        bullet->SetHoming(true);
        bullet->SetHomingStrength(s.homingStrength);
        if (s.bulletSpeed <= 260.0f) {
            bullet->SetCollisionSize(16.0f, 16.0f);
        }
    }
    if (s.pattern == WeaponPattern::LASER) {
        bullet->SetCollisionSize(12.0f, 12.0f);
    }
    return bullet;
}

void GenericWeapon::Fire(const Vector2& position, const Vector2& direction,
                         BulletFaction faction, EntityManager& entityMgr) {
    const WeaponSpec& s = GetWeaponSpec(m_type);
    Vector2 aim = direction.Normalized();
    if (aim.LengthSquared() < 0.001f) aim = Vector2(1.0f, 0.0f);
    float baseAngle = std::atan2(aim.y, aim.x);
    int count = (std::max)(1, m_projectileCount);

    switch (s.pattern) {
        case WeaponPattern::SPREAD:
        case WeaponPattern::CODEX:
        case WeaponPattern::FLAME:
        case WeaponPattern::ORBITAL: {
            float spread = DegToRad(m_spreadAngle);
            for (int i = 0; i < count; ++i) {
                float offset = 0.0f;
                if (s.pattern == WeaponPattern::ORBITAL) {
                    offset = (2.0f * PI / count) * i;
                } else if (s.pattern == WeaponPattern::FLAME) {
                    offset = DegToRad(RandomFloat(-m_spreadAngle, m_spreadAngle));
                } else {
                    float t = (count > 1) ? (float)i / (count - 1) : 0.5f;
                    offset = -spread * 0.5f + spread * t;
                }
                Vector2 dir(std::cos(baseAngle + offset), std::sin(baseAngle + offset));
                float speedScale = s.pattern == WeaponPattern::FLAME ? RandomFloat(0.85f, 1.15f) : 1.0f;
                SpawnSpecBullet(position, dir, faction, entityMgr, speedScale, 1.0f);
            }
            break;
        }
        case WeaponPattern::BURST: {
            for (int i = 0; i < count; ++i) {
                float offset = DegToRad(((float)i - (count - 1) * 0.5f) * (m_spreadAngle / (std::max)(1, count)));
                Vector2 dir(std::cos(baseAngle + offset), std::sin(baseAngle + offset));
                Vector2 side(-aim.y, aim.x);
                Vector2 pos = position + side * (((float)i - (count - 1) * 0.5f) * 5.0f);
                SpawnSpecBullet(pos, dir, faction, entityMgr, 1.0f + i * 0.03f, 1.0f);
            }
            break;
        }
        case WeaponPattern::HOMING: {
            for (int i = 0; i < count; ++i) {
                float t = (count > 1) ? (float)i / (count - 1) : 0.5f;
                float offset = (t - 0.5f) * DegToRad((std::max)(8.0f, m_spreadAngle));
                Vector2 dir(std::cos(baseAngle + offset), std::sin(baseAngle + offset));
                SpawnSpecBullet(position, dir, faction, entityMgr, 1.0f, 1.0f);
            }
            break;
        }
        case WeaponPattern::LASER: {
            Vector2 side(-aim.y, aim.x);
            int beams = (std::max)(1, count);
            int segments = s.tier >= 3 ? 15 : 12;
            float spacing = s.tier >= 3 ? 58.0f : 54.0f;
            float beamOffset = beams > 1 ? 18.0f : 0.0f;
            for (int b = 0; b < beams; ++b) {
                float lane = ((float)b - (beams - 1) * 0.5f) * beamOffset;
                Vector2 laneOrigin = position + side * lane + aim * 32.0f;
                for (int i = 0; i < segments; ++i) {
                    Vector2 pos = laneOrigin + aim * (i * spacing);
                    if (pos.x < 40.0f || pos.x > ROOM_WIDTH - 40.0f ||
                        pos.y < 40.0f || pos.y > ROOM_HEIGHT - 40.0f) {
                        break;
                    }
                    Bullet* beam = CreateBullet(pos, aim, faction, entityMgr);
                    beam->SetSpeed(0.0f);
                    beam->SetLifetime(0.09f);
                    beam->SetColor(s.color);
                    beam->SetPiercing(true);
                    beam->SetMaxPierceCount(s.maxPierce > 0 ? s.maxPierce : 4);
                    beam->SetCollisionSize(s.tier >= 3 ? 19.0f : 16.0f,
                                           s.tier >= 3 ? 19.0f : 16.0f);
                    beam->SetCanBeBlocked(false);
                    if (s.burning) {
                        beam->SetCausesBurning(true);
                        beam->SetBurnChance(0.4f);
                        beam->SetBurnDuration(2.0f);
                    }
                }
            }
            break;
        }
        case WeaponPattern::BOUNCE:
        case WeaponPattern::EXPLOSIVE:
        case WeaponPattern::SINGLE:
        default: {
            float angleOffset = m_spreadAngle > 0.0f ? DegToRad(RandomFloat(-m_spreadAngle, m_spreadAngle)) : 0.0f;
            Vector2 dir(std::cos(baseAngle + angleOffset), std::sin(baseAngle + angleOffset));
            SpawnSpecBullet(position, dir, faction, entityMgr, 1.0f, 1.0f);
            break;
        }
    }
}
