#pragma once

#include "Character.h"
#include "../Weapon/Weapon.h"

// ============================================================
// Player - 玩家类，处理输入、技能、武器切换
// ============================================================
class Player : public Character {
public:
    Player();
    virtual ~Player();

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Player"; }

    virtual void TakeDamage(int damage) override;
    virtual void ApplyBurning(float duration) override;
    virtual void ApplyFrozen(float duration) override;
    virtual void Die() override;

    // 初始化职业属性
    void InitProfession(Profession prof);
    Profession GetProfession() const { return m_profession; }

    // 武器系统
    bool EquipWeapon(WeaponType type);
    void FireWeapon();
    void SwitchWeapon(int slot);  // 切换武器槽
    Weapon* GetCurrentWeapon()      { return m_currentWeapon; }
    Weapon* GetWeapon(int slot)     { return m_weapons[slot].get(); }
    int     GetWeaponCount() const  { return m_weaponCount; }
    bool    AddWeapon(std::unique_ptr<Weapon> weapon);  // 拾取新武器

    // 武器满时暂存待替换
    bool    HasPendingWeapon() const { return m_pendingWeapon != nullptr; }
    Weapon* GetPendingWeapon()       { return m_pendingWeapon.get(); }
    WeaponType GetPendingWeaponType() const { return m_pendingWeaponType; }
    void    ClearPendingWeapon()     { m_pendingWeapon.reset(); }
    std::unique_ptr<Weapon> TakePendingWeapon() { return std::move(m_pendingWeapon); }

    // 替换指定槽位的武器
    void    ReplaceWeapon(int slot, std::unique_ptr<Weapon> newWeapon);

    // 技能
    void UseSkill();
    float GetSkillCooldown() const { return m_skillCooldown; }
    float GetSkillCooldownRemaining() const { return m_skillCooldownRemaining; }
    bool  IsSkillReady() const { return m_skillCooldownRemaining <= 0.0f; }
    void  ReduceSkillCooldown(float percent);  // CD缩减 buff

    // 输入处理
    void ProcessKeyboardInput(float deltaTime);
    void ProcessMouseInput();

    // 瞄准
    const Vector2& GetAimDirection() const { return m_aimDirection; }
    float GetAimAngle() const { return m_aimAngle; }

    // 翻滚（游侠技能）
    bool IsRolling() const { return m_isRolling; }
    bool IsRollInvincible() const { return m_isRolling && m_rollInvincible; }
    bool IsFirstShotAfterRoll() const { return m_firstShotAfterRoll; }
    void ConsumeFirstShotAfterRoll() { m_firstShotAfterRoll = false; }

    // 骑士技能状态
    bool IsFullFire() const { return m_fullFireActive; }

    // 金币
    int  GetGold() const { return m_gold; }
    void AddGold(int amount) { m_gold += amount; }

    // 复活
    bool HasReviveCharm() const { return m_hasReviveCharm; }
    void SetReviveCharm(bool has) { m_hasReviveCharm = has; }
    void OnRevive();

private:
    void UpdateSkillCooldown(float deltaTime);
    void UpdateFullFire(float deltaTime);
    void UpdateRoll(float deltaTime);

    Profession  m_profession;

    // 武器
    static constexpr int MAX_WEAPON_SLOTS = 2;
    std::unique_ptr<Weapon> m_weapons[MAX_WEAPON_SLOTS];
    Weapon*                 m_currentWeapon;
    int                     m_weaponCount;

    // 待替换武器（武器槽满时暂存）
    std::unique_ptr<Weapon> m_pendingWeapon;
    WeaponType              m_pendingWeaponType;

    // 技能
    float       m_skillCooldown;            // 基础冷却时间(秒)
    float       m_skillCooldownRemaining;   // 剩余冷却
    float       m_skillCooldownModifier;    // CD缩减倍率 (1.0 = 100%)

    // 骑士技能：火力全开
    bool        m_fullFireActive;
    float       m_fullFireTimer;

    // 游侠技能：战术翻滚
    bool        m_isRolling;
    bool        m_rollInvincible;
    bool        m_firstShotAfterRoll;
    Vector2     m_rollDirection;
    float       m_rollTimer;
    float       m_rollDistance;

    // 法师技能：连锁闪电特效
    float                   m_lightningVfxTimer;
    std::vector<Vector2>    m_lightningChainPositions;  // 连锁闪电的位置链

    // MP 回复已移除，依赖掉落药水

    // 瞄准
    Vector2     m_aimDirection;
    float       m_aimAngle;

    // 输入
    bool        m_keyW, m_keyA, m_keyS, m_keyD;
    bool        m_keySpace;
    bool        m_keyWeaponPrev[MAX_WEAPON_SLOTS];  // 武器切换键上一帧状态
    bool        m_mouseLeft;
    bool        m_mouseLeftPrev;   // 上一帧的左键状态
    bool        m_fireLocked;      // 输入锁：需松开鼠标才能开火

    // 金币
    int         m_gold;

    // 复活
    bool        m_hasReviveCharm;

    // 无敌帧（受伤后的短暂无敌）
    float       m_damageInvincibleTimer;
    static constexpr float DAMAGE_INVINCIBLE_DURATION = 0.6f;
};
