#pragma once

#include "GameObject.h"

// ============================================================
// Character - 可移动、有属性的角色基类（玩家 / 敌人 / Boss 的公共父类）
// ============================================================
class Character : public GameObject {
public:
    Character();
    virtual ~Character() = default;

    virtual void Update(float deltaTime) override = 0;
    virtual void Render() override = 0;
    virtual const char* GetTypeName() const override { return "Character"; }

    // 受到伤害（虚函数，允许派生类覆写以触发无敌帧等）
    virtual void TakeDamage(int damage);
    virtual void Heal(int amount);
    virtual void Die();
    bool IsDead() const { return m_isDead; }

    // 属性访问
    int  GetHP()        const { return m_hp; }
    int  GetMaxHP()     const { return m_maxHP; }
    int  GetMP()        const { return m_mp; }
    int  GetMaxMP()     const { return m_maxMP; }
    float GetMoveSpeed() const { return m_moveSpeed; }
    int  GetArmor()     const { return m_armor; }
    int  GetShield()    const { return m_shield; }
    int  GetMaxShield() const { return m_maxShield; }

    void SetHP(int hp);
    void SetMaxHP(int maxHP);
    void SetMP(int mp);
    void SetMaxMP(int maxMP);
    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetArmor(int armor) { m_armor = armor; }
    void SetShield(int shield);
    void SetMaxShield(int maxShield);
    void RestoreShield(int amount);

    // 状态效果
    bool IsStunned()         const { return m_isStunned; }
    bool IsBurning()         const { return m_isBurning; }
    bool IsFrozen()          const { return m_isFrozen; }
    bool IsInvincible()      const { return m_isInvincible; }

    void SetStunned(bool v)      { m_isStunned = v; }
    void SetBurning(bool v)      { m_isBurning = v; }
    void SetFrozen(bool v)       { m_isFrozen = v; }
    void SetInvincible(bool v)   { m_isInvincible = v; }

    // 施加/移除状态效果（含持续时间和 Tick 逻辑）
    virtual void ApplyBurning(float duration);
    virtual void ApplyFrozen(float duration);
    void ApplyStun(float duration);
    void SetInvincibleDuration(float duration);

    // 移动方向
    const Vector2& GetMoveDirection() const { return m_moveDirection; }
    void SetMoveDirection(const Vector2& dir) { m_moveDirection = dir; }

protected:
    virtual void UpdateStatusEffects(float deltaTime);

    int     m_hp;
    int     m_maxHP;
    int     m_mp;
    int     m_maxMP;
    float   m_moveSpeed;
    int     m_armor;             // 护甲（减伤值）

    // 护盾（承受伤害优先于 HP，脱战 2 秒后每秒恢复 1 点）
    int     m_shield;
    int     m_maxShield;
    float   m_shieldRegenTimer;   // 距离上次受伤经过的时间

    bool    m_isDead;

    // 移动
    Vector2 m_moveDirection;

    // 状态异常标记
    bool    m_isStunned;
    bool    m_isBurning;
    bool    m_isFrozen;
    bool    m_isInvincible;

    // 状态异常计时器
    float   m_stunTimer;
    float   m_burningTimer;
    float   m_burningTickTimer;   // 燃烧扣血间隔
    float   m_frozenTimer;
    float   m_invincibleTimer;

    float   m_origMoveSpeed;    // 冰冻前的原始速度，用于恢复
};
