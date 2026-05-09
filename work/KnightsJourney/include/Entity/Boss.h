#pragma once

#include "Character.h"

class Player;

// ============================================================
// Boss - Boss 基类，含阶段切换和特殊技能模式
// ============================================================
class Boss : public Character {
public:
    Boss();
    virtual ~Boss() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Boss"; }

    virtual void TakeDamage(int damage) override;
    virtual void Die() override;

    // Boss 阶段
    BossPhase GetPhase() const { return m_phase; }
    void SetPhase(BossPhase phase);
    float GetPhaseThreshold() const;   // 根据当前阶段返回 HP 百分比阈值

    // 技能系统（纯虚函数，每个 Boss 自定义）
    virtual void CastSkill1() = 0;
    virtual void CastSkill2() = 0;

    // 技能冷却
    bool CanCastSkill1() const { return m_skill1Timer <= 0.0f; }
    bool CanCastSkill2() const { return m_skill2Timer <= 0.0f; }

    // 目标
    void SetTarget(Player* target) { m_pTarget = target; }
    Player* GetTarget() const { return m_pTarget; }

    // Boss 血条显示名称
    virtual const char* GetBossName() const = 0;

protected:
    virtual void UpdateSkillTimers(float deltaTime);
    virtual void UpdatePhaseBehavior(float deltaTime);

    BossPhase   m_phase;
    Player*     m_pTarget;

    // 技能冷却
    float       m_skill1Cooldown;
    float       m_skill1Timer;
    float       m_skill2Cooldown;
    float       m_skill2Timer;

    // 技能计时器（用于持续性技能）
    float       m_skillCastTimer;     // 当前技能施放中的计时
    bool        m_isCasting;          // 是否正在施放技能

    // 半血蓝量补给（只触发一次）
    bool        m_halfHpMpDropped;
};

// ============================================================
// 森林 Boss - 巨型树人 (Treant)
// 技能1 战争践踏：产生一圈向外扩散的震荡波弹幕
// 技能2 召唤：召唤 3 只哥布林近战
// ============================================================
class Treant : public Boss {
public:
    Treant();
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Treant"; }
    virtual const char* GetBossName() const override { return "巨型树人"; }

    virtual void CastSkill1() override;  // 战争践踏
    virtual void CastSkill2() override;  // 召唤哥布林

private:
    int     m_shockwaveProjectileCount;   // 震荡波弹幕数量
    float   m_shockwaveRadius;            // 震荡波扩散半径
    float   m_shockwaveSpeed;
    int     m_summonCount;                // 每轮召唤数量
};

// ============================================================
// 冰原 Boss - 水晶巨蟹 (Crystal Crab)
// 技能1 交叉激光扫射
// 技能2 缩壳回血（玩家攻击反而回血）
// ============================================================
class CrystalCrab : public Boss {
public:
    CrystalCrab();
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void TakeDamage(int damage) override;
    virtual const char* GetTypeName() const override { return "CrystalCrab"; }
    virtual const char* GetBossName() const override { return "水晶巨蟹"; }

    virtual void CastSkill1() override;  // 激光扫射
    virtual void CastSkill2() override;  // 缩壳回血

    bool IsShelled() const { return m_isShelled; }

private:
    bool    m_isShelled;            // 缩壳状态
    float   m_shellTimer;
    float   m_shellHealPerSecond;
    float   m_laserAngle;
    float   m_laserRotateSpeed;
    int     m_laserCount;           // 激光束数量
};

// ============================================================
// 熔岩 Boss - 熔岩飞龙 (Lava Dragon)
// 技能1 扇形火焰喷吐（全屏弹幕）
// 技能2 潜地追踪 — 钻入地下追踪玩家，地面可见移动轨迹，钻出时范围伤害
// ============================================================
class LavaDragon : public Boss {
public:
    LavaDragon();
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void TakeDamage(int damage) override;
    virtual const char* GetTypeName() const override { return "LavaDragon"; }
    virtual const char* GetBossName() const override { return "熔岩飞龙"; }

    virtual void CastSkill1() override;  // 火焰喷吐
    virtual void CastSkill2() override;  // 潜地追踪

    bool IsBurrowed() const { return m_isBurrowed; }

private:
    // 潜地状态
    bool    m_isBurrowed;
    float   m_burrowTimer;           // 潜地剩余时间
    float   m_burrowTrailTimer;      // 轨迹生成间隔计时
    Vector2 m_burrowTargetPos;       // 追踪目标位置（实时更新）
    std::vector<Vector2> m_burrowTrail;  // 地面轨迹点
    float   m_burrowEmergeRadius;    // 钻出伤害半径
    int     m_burrowEmergeDamage;    // 钻出伤害

    // 火焰喷吐参数
    int     m_fireBreathProjectileCount;
    float   m_fireBreathSpreadAngle;
};
