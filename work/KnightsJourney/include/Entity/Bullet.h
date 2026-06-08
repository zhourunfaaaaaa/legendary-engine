#pragma once

#include "GameObject.h"

// ============================================================
// Bullet - 子弹/弹幕基类
// 支持多种弹道行为：直线、追踪、反弹、穿透、爆炸
// ============================================================
class Bullet : public GameObject {
public:
    Bullet();
    virtual ~Bullet() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Bullet"; }

    // 子弹属性
    BulletFaction GetFaction()     const { return m_faction; }
    void SetFaction(BulletFaction f) { m_faction = f; }
    bool IsPlayerBullet() const { return m_faction == BulletFaction::PLAYER; }
    bool IsEnemyBullet() const { return m_faction == BulletFaction::ENEMY; }

    int  GetDamage() const { return m_damage; }
    void SetDamage(int dmg) { m_damage = dmg; }

    float GetSpeed() const { return m_speed; }
    void SetSpeed(float spd) { m_speed = spd; }
    void SetInheritedForwardSpeed(float spd) { m_inheritedForwardSpeed = spd > 0.0f ? spd : 0.0f; }
    float GetInheritedForwardSpeed() const { return m_inheritedForwardSpeed; }

    void SetDirection(const Vector2& dir) { m_direction = dir.Normalized(); }
    void SetDirection(float angleRad);
    const Vector2& GetDirection() const { return m_direction; }

    // 特殊弹道属性
    bool IsPiercing()     const { return m_piercing; }
    bool IsHoming()       const { return m_homing; }
    bool IsBouncing()     const { return m_bouncing; }
    bool IsExplosive()    const { return m_explosive; }
    bool IsMeleeSlash()   const { return m_isMeleeSlash; }

    void SetPiercing(bool v)  { m_piercing = v; }
    void SetHoming(bool v)    { m_homing = v; }
    void SetBouncing(bool v)  { m_bouncing = v; }
    void SetExplosive(bool v) { m_explosive = v; }
    void SetMeleeSlash(bool v) { m_isMeleeSlash = v; }

    // 追踪目标
    void SetHomingTarget(GameObject* target) { m_homingTarget = target; }
    GameObject* GetHomingTarget() const { return m_homingTarget; }
    float GetHomingStrength() const { return m_homingStrength; }
    void SetHomingStrength(float s) { m_homingStrength = s; }

    // 反弹
    int  GetMaxBounces() const { return m_maxBounces; }
    int  GetBounceCount() const { return m_bounceCount; }
    void IncrementBounce() { m_bounceCount++; }
    bool CanBounce() const { return m_bounceCount < m_maxBounces; }
    void SetMaxBounces(int max) { m_maxBounces = max; }

    // 爆炸
    float GetExplosionRadius() const { return m_explosionRadius; }
    void SetExplosionRadius(float r) { m_explosionRadius = r; }

    // 穿透计数（某些子弹只能穿透有限次数）
    int  GetMaxPierceCount() const { return m_maxPierceCount; }
    int  GetPierceCount() const { return m_pierceCount; }
    void IncrementPierce() { m_pierceCount++; }
    bool CanPierce() const { return m_pierceCount < m_maxPierceCount; }
    void SetMaxPierceCount(int max) { m_maxPierceCount = max; }

    // 穿透子弹已命中目标追踪（防止同一目标被多帧重复命中）
    bool HasHitTarget(int id) const {
        for (int i = 0; i < m_hitTargetCount; ++i)
            if (m_hitTargetIDs[i] == id) return true;
        return false;
    }
    void AddHitTarget(int id) {
        if (m_hitTargetCount < 8) m_hitTargetIDs[m_hitTargetCount++] = id;
    }

    // 燃烧效果
    bool CausesBurning() const { return m_causesBurning; }
    void SetCausesBurning(bool v) { m_causesBurning = v; }
    float GetBurnChance() const { return m_burnChance; }
    void SetBurnChance(float chance) { m_burnChance = chance; }
    float GetBurnDuration() const { return m_burnDuration; }
    void SetBurnDuration(float dur) { m_burnDuration = dur; }

    // 减速效果
    bool CausesSlow() const { return m_causesSlow; }
    void SetCausesSlow(bool v) { m_causesSlow = v; }
    float GetSlowDuration() const { return m_slowDuration; }
    void SetSlowDuration(float dur) { m_slowDuration = dur; }

    // 吸血
    bool HasLifeSteal() const { return m_hasLifeSteal; }
    void SetLifeSteal(bool v) { m_hasLifeSteal = v; }
    float GetLifeStealChance() const { return m_lifeStealChance; }
    void SetLifeStealChance(float chance) { m_lifeStealChance = chance; }

    // 寿命
    float GetLifetime() const { return m_lifetime; }
    void SetLifetime(float lt) { m_lifetime = lt; }

    // 可被光剑抵消
    bool CanBeBlocked() const { return m_canBeBlocked; }
    void SetCanBeBlocked(bool v) { m_canBeBlocked = v; }

    // 武器类型来源
    WeaponType GetSourceWeapon() const { return m_sourceWeapon; }
    void SetSourceWeapon(WeaponType wt) { m_sourceWeapon = wt; }

    // 暴击
    bool IsCritical() const { return m_isCritical; }
    void SetCritical(bool v) { m_isCritical = v; if (v) m_damage *= 2; }

protected:
    BulletFaction m_faction;
    int           m_damage;
    float         m_speed;
    float         m_inheritedForwardSpeed;
    Vector2       m_direction;

    // 特殊弹道属性
    bool          m_piercing;
    bool          m_homing;
    bool          m_bouncing;
    bool          m_explosive;
    bool          m_isMeleeSlash;     // 光剑刀光

    // 追踪
    GameObject*   m_homingTarget;
    float         m_homingStrength;

    // 反弹
    int           m_maxBounces;
    int           m_bounceCount;

    // 穿透
    int           m_maxPierceCount;
    int           m_pierceCount;
    int           m_hitTargetIDs[8];
    int           m_hitTargetCount;

    // 爆炸
    float         m_explosionRadius;

    // 效果
    bool          m_causesBurning;
    float         m_burnChance;        // 0.0 ~ 1.0
    float         m_burnDuration;

    bool          m_causesSlow;
    float         m_slowDuration;

    bool          m_hasLifeSteal;
    float         m_lifeStealChance;

    // 通用
    float         m_lifetime;
    float         m_age;
    bool          m_canBeBlocked;      // 是否可被光剑抵消
    WeaponType    m_sourceWeapon;
    bool          m_isCritical;
    unsigned int  m_color;             // EasyX 绘制颜色

public:
    void SetColor(unsigned int c) { m_color = c; }
    unsigned int GetColor() const { return m_color; }
};

// ============================================================
// ExplosionEffect - 爆炸视觉特效
// ============================================================
class ExplosionEffect : public GameObject {
public:
    ExplosionEffect();
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "ExplosionEffect"; }

    void SetMaxRadius(float r) { m_maxRadius = r; }

private:
    float m_maxRadius;
    float m_currentRadius;
    float m_lifetime;
    float m_age;
};
