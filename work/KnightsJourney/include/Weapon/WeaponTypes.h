#pragma once

#include "Weapon.h"

// ============================================================
// 1. 突击步枪 (AssaultRifle)
// MP: 1, 射速: 极快, 轻微随机散射角度
// ============================================================
class AssaultRifle : public Weapon {
public:
    AssaultRifle();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "突击步枪"; }
    virtual WeaponType GetType() const override { return WeaponType::ASSAULT_RIFLE; }
    virtual const char* GetDescription() const override { return "极快射速，轻微精度偏移"; }

private:
    float m_accuracyOffset;   // 随机偏移量
};

// ============================================================
// 3. 霰弹枪 (Shotgun)
// MP: 3, 射速: 慢, 一次扇形发射 5 发子弹
// ============================================================
class Shotgun : public Weapon {
public:
    Shotgun();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "霰弹枪"; }
    virtual WeaponType GetType() const override { return WeaponType::SHOTGUN; }
    virtual const char* GetDescription() const override { return "扇形发射5发子弹，近距离威力巨大"; }

protected:
    virtual void OnUpgrade() override;   // 天赋可提升至 7 发
};

// ============================================================
// 4. 狙击枪 (SniperRifle)
// MP: 4, 射速: 极慢, 子弹速度极快, 穿透属性
// ============================================================
class SniperRifle : public Weapon {
public:
    SniperRifle();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "狙击枪"; }
    virtual WeaponType GetType() const override { return WeaponType::SNIPER_RIFLE; }
    virtual const char* GetDescription() const override { return "穿透子弹，极高伤害，极慢射速"; }

private:
    static constexpr float BULLET_SPEED = 2000.0f;  // 极快弹速
};

// ============================================================
// 5. 火箭筒 (RocketLauncher)
// MP: 5, 射速: 慢, AOE 爆炸范围伤害
// ============================================================
class RocketLauncher : public Weapon {
public:
    RocketLauncher();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "火箭筒"; }
    virtual WeaponType GetType() const override { return WeaponType::ROCKET_LAUNCHER; }
    virtual const char* GetDescription() const override { return "AOE爆炸伤害，击中敌人或墙壁后爆炸"; }

    float GetExplosionRadius() const { return m_explosionRadius; }
    void  SetExplosionRadius(float r) { m_explosionRadius = r; }  // 火盾 buff 可扩大

private:
    float m_explosionRadius;
    float m_rocketSpeed;  // 火箭弹飞行速度（较慢）
};

// ============================================================
// 7. 火焰机枪 (FlameThrower)
// MP: 2, 密集短程火苗, 30% 点燃
// ============================================================
class FlameThrower : public Weapon {
public:
    FlameThrower();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "火焰机枪"; }
    virtual WeaponType GetType() const override { return WeaponType::FLAME_THROWER; }
    virtual const char* GetDescription() const override { return "短程密集火焰，30%几率点燃敌人"; }

private:
    float   m_flameRange;         // 火焰射程
    float   m_burnChance;         // 30% 点燃概率
    float   m_burnDuration;       // 点燃持续 3 秒
    int     m_flameCountPerShot;  // 每次发射火苗数
};

// ============================================================
// 8. 魔法法杖 (MagicStaff)
// MP: 3, 发射 2 枚追踪魔法弹
// ============================================================
class MagicStaff : public Weapon {
public:
    MagicStaff();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "魔法法杖"; }
    virtual WeaponType GetType() const override { return WeaponType::MAGIC_STAFF; }
    virtual const char* GetDescription() const override { return "发射自动追踪敌人的魔法弹"; }

    float GetHomingStrength() const { return m_homingStrength; }

protected:
    virtual void OnUpgrade() override;   // 天赋可提升至 3 枚追踪弹

private:
    float m_homingStrength;      // 追踪力度
    float m_magicBallSpeed;
};

// ============================================================
// 9. 反弹十字弩 (ReboundCrossbow)
// MP: 2, 弩箭碰到墙壁后镜面反弹，最多 3 次
// ============================================================
class ReboundCrossbow : public Weapon {
public:
    ReboundCrossbow();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "反弹十字弩"; }
    virtual WeaponType GetType() const override { return WeaponType::REBOUND_CROSSBOW; }
    virtual const char* GetDescription() const override { return "弩箭碰到墙壁后反弹，最多3次"; }

private:
    int     m_maxBounces;
    float   m_boltSpeed;
};

// ============================================================
// 10. 吸血鬼法典 (VampireCodex)
// MP: 4, 蝙蝠子弹击杀敌人 20% 概率回复 1 HP
// ============================================================
class VampireCodex : public Weapon {
public:
    VampireCodex();
    virtual void Fire(const Vector2& position, const Vector2& direction,
                      BulletFaction faction, EntityManager& entityMgr) override;
    virtual const char* GetName() const override { return "吸血鬼法典"; }
    virtual WeaponType GetType() const override { return WeaponType::VAMPIRE_CODEX; }
    virtual const char* GetDescription() const override { return "蝙蝠子弹，击杀敌人20%概率回复1HP"; }

private:
    float   m_lifeStealChance;   // 20%
    int     m_lifeStealAmount;   // 1 HP
    float   m_batSpeed;
};
