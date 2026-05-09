#pragma once

#include "../Core/Common.h"
#include "../Entity/Bullet.h"

class Player;
class EntityManager;

// ============================================================
// Weapon - 武器抽象基类
// 每种武器有自己的开火逻辑 (Fire)、弹道逻辑和资源消耗
// ============================================================
class Weapon {
public:
    Weapon();
    virtual ~Weapon() = default;

    // 开火（纯虚函数，每个武器自定义）
    // 返回生成的子弹列表
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) = 0;

    // 是否可以开火（检查冷却和 MP）
    virtual bool CanFire(Player* player) const;

    // 获取武器信息
    virtual const char* GetName() const = 0;
    virtual WeaponType GetType() const = 0;
    virtual const char* GetDescription() const { return ""; }

    int  GetMPCost() const { return m_mpCost; }
    float GetFireRate() const { return m_fireRate; }    // 每秒射速
    float GetCooldownRemaining() const { return m_cooldownTimer; }
    bool  IsOnCooldown() const { return m_cooldownTimer > 0.0f; }

    // 更新冷却
    virtual void UpdateCooldown(float deltaTime);

    // 武器特殊效果修改器（天赋 buff 可能修改）
    int   GetBaseDamage() const { return m_baseDamage; }
    void  SetBaseDamage(int dmg) { m_baseDamage = dmg; }

    float GetSpreadAngle() const { return m_spreadAngle; }
    void  SetSpreadAngle(float angle) { m_spreadAngle = angle; }

    float GetCritChance() const { return m_critChance; }
    void  SetCritChance(float chance) { m_critChance = chance; }

    int   GetProjectileCount() const { return m_projectileCount; }
    void  SetProjectileCount(int count) { m_projectileCount = count; }

    // 武器等级（某些武器可升级）
    int   GetLevel() const { return m_level; }
    void  Upgrade() { m_level++; OnUpgrade(); }

    // 价格（商店）
    int   GetPrice() const { return m_price; }

    // 射速修改（骑士技能 / buff）
    void  SetFireRateMultiplier(float mult) { m_fireRateMultiplier = mult; }
    float GetFireRateMultiplier() const { return m_fireRateMultiplier; }

protected:
    virtual void OnUpgrade() {}

    // 创建一颗基础子弹的工厂方法
    Bullet* CreateBullet(const Vector2& position, const Vector2& direction,
                         BulletFaction faction, EntityManager& entityMgr);

    int     m_mpCost;            // MP 消耗
    float   m_fireRate;          // 基础射速 (每秒发数)
    float   m_cooldownTimer;     // 当前冷却剩余
    float   m_fireRateMultiplier;// 射速倍率 (1.0 = 正常, 2.0 = 双倍)
    int     m_baseDamage;        // 基础伤害
    float   m_spreadAngle;       // 散射角度 (度)
    float   m_critChance;        // 暴击率
    int     m_projectileCount;   // 每次开火子弹数
    int     m_level;             // 武器等级
    int     m_price;             // 商店价格
};
