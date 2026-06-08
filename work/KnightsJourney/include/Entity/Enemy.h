#pragma once

#include "Character.h"

class Player;

// ============================================================
// Enemy - 怪物基类，包含 AI 状态机和仇恨系统
// ============================================================
class Enemy : public Character {
public:
    Enemy();
    virtual ~Enemy() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Enemy"; }

    virtual void TakeDamage(int damage) override;
    virtual void Die() override;

    // AI 状态机
    EnemyState GetAIState() const { return m_aiState; }
    void SetAIState(EnemyState state);
    virtual void UpdateAI(float deltaTime);

    // 攻击
    virtual void Attack();
    bool CanAttack() const { return m_attackTimer <= 0.0f; }
    void FireBullet(float speed = 250.0f, unsigned int color = 0xFF4444, int bulletDamage = -1);  // 发射子弹，bulletDamage<0 则用 m_damage

    // 目标追踪
    void SetTarget(Player* target) { m_pTarget = target; }
    Player* GetTarget() const { return m_pTarget; }
    float GetDistanceToTarget() const;
    Vector2 GetDirectionToTarget() const;
    bool IsTargetInRange(float range) const;

    // 掉落物
    int  GetGoldDrop() const { return m_goldDrop; }
    int  GetExpDrop() const { return m_expDrop; }

    // 精英属性
    bool IsElite() const { return m_isElite; }
    void SetElite(bool elite);

    // 适用关卡
    BiomeType GetBiome() const { return m_biome; }

protected:
    virtual void UpdatePatrol(float deltaTime);
    virtual void UpdateChase(float deltaTime);
    virtual void UpdateAttack(float deltaTime);

    EnemyState  m_aiState;
    Player*     m_pTarget;

    // 战斗属性
    int         m_damage;
    float       m_attackRange;
    float       m_attackCooldown;
    float       m_attackTimer;
    float       m_detectionRange;       // 仇恨检测范围

    // 巡逻
    Vector2     m_patrolOrigin;
    float       m_patrolRadius;
    float       m_patrolAngle;
    float       m_patrolSpeed;

    // 掉落
    int         m_goldDrop;
    int         m_expDrop;

    // 精英标记
    bool        m_isElite;
    float       m_eliteScale;           // 1.5 倍体型

    // 所属关卡
    BiomeType   m_biome;

    // 受伤闪烁
    float       m_damageFlashTimer;

    // 闲逛状态下的随机运动
    float       m_idleWanderTimer;
    Vector2     m_idleWanderDir;

    // 战斗 AI 的短周期决策缓存
    float       m_aiDecisionTimer;
    Vector2     m_tacticalDir;
};

// ============================================================
// 哥布林近战 (Forest) - 直奔玩家
// ============================================================
class GoblinMelee : public Enemy {
public:
    GoblinMelee();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "GoblinMelee"; }
};

// ============================================================
// 哥布林弓箭手 (Forest) - 保持距离射箭
// ============================================================
class GoblinArcher : public Enemy {
public:
    GoblinArcher();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "GoblinArcher"; }

private:
    float m_preferredDistance;   // 与玩家保持的偏好距离
};

// ============================================================
// 冲锋野猪 (Forest) - 蓄力后直线冲撞
// ============================================================
class ChargeBoar : public Enemy {
public:
    ChargeBoar();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "ChargeBoar"; }

private:
    enum class BoarState { IDLE, CHARGING, RUSHING, STUNNED_AFTER_WALL };
    BoarState m_boarState;
    float     m_chargeTimer;
    float     m_rushTimer;
    float     m_stunTimer;
    Vector2   m_rushDirection;
    float     m_rushSpeed;
};

// ============================================================
// 森林幽灯 (Forest) - 绕身移动并发射孢子弹
// ============================================================
class ForestWisp : public Enemy {
public:
    ForestWisp();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "ForestWisp"; }

private:
    float m_orbitAngle;
    float m_preferredDistance;
};

// ============================================================
// 冰原史莱姆 (Ice Dungeon) - 死亡分裂
// ============================================================
class IceSlime : public Enemy {
public:
    IceSlime();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual void Die() override;  // 死亡分裂逻辑
    virtual const char* GetTypeName() const override { return "IceSlime"; }

    bool IsSmallSlime() const { return m_isSmall; }
    void SetAsSmallSlime();

private:
    bool m_isSmall;              // 是否是小史莱姆（不再次分裂）
    static constexpr int SPLIT_COUNT = 2;
};

// ============================================================
// 雪人 (Ice Dungeon) - 投掷减速雪球
// ============================================================
class Snowman : public Enemy {
public:
    Snowman();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Snowman"; }

    // 雪球减速持续时间
    static float GetSlowDuration() { return 3.0f; }
};

// ============================================================
// 寒冰法师 (Ice Dungeon) - 静止发射扇形冰刺
// ============================================================
class IceMage : public Enemy {
public:
    IceMage();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "IceMage"; }

private:
    float   m_burstTimer;
    int     m_burstCount;          // 一轮发射几发
    float   m_burstInterval;       // 每发间隔
    float   m_burstAngleSpread;    // 扇形角度
};

// ============================================================
// 霜刃游卫 (Ice Dungeon) - 高速斜切并发射减速冰刃
// ============================================================
class FrostScout : public Enemy {
public:
    FrostScout();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "FrostScout"; }

private:
    float m_skateTimer;
    float m_sideSign;
};

// ============================================================
// 熔岩虫 (Volcano) - 留下火焰轨迹
// ============================================================
class LavaWorm : public Enemy {
public:
    LavaWorm();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "LavaWorm"; }

private:
    float   m_trailTimer;          // 留下轨迹的间隔
    float   m_trailDuration;       // 轨迹持续 3 秒
    std::vector<Vector2> m_trailPositions;
};

// ============================================================
// 火焰骑士 (Volcano) - 正面盾牌免疫
// ============================================================
class FireKnight : public Enemy {
public:
    FireKnight();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual void TakeDamage(int damage) override;
    virtual const char* GetTypeName() const override { return "FireKnight"; }

    bool IsShieldUp() const { return m_shieldUp; }
    Vector2 GetFacingDirection() const { return m_facingDirection; }

private:
    bool    m_shieldUp;
    Vector2 m_facingDirection;
    float   m_shieldAngle;        // 盾牌格挡角度
};

// ============================================================
// 自爆蝙蝠 (Volcano) - 飞行靠近后自爆
// ============================================================
class ExplosiveBat : public Enemy {
public:
    ExplosiveBat();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual void Die() override;   // 死亡自爆
    virtual const char* GetTypeName() const override { return "ExplosiveBat"; }

    float GetExplosionRadius() const { return m_explosionRadius; }
    bool  IsExploding() const { return m_isExploding; }

private:
    float   m_explosionRadius;
    bool    m_isExploding;
    float   m_explosionTimer;
    bool    m_ignoreObstacles;     // 无视障碍物
};

// ============================================================
// 余烬小鬼 (Volcano) - 机动走位并发射成对火弹
// ============================================================
class EmberImp : public Enemy {
public:
    EmberImp();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "EmberImp"; }

private:
    float m_phaseTimer;
    float m_zigzag;
};

// ============================================================
// 伪装宝箱怪 (通用) - 奖励房伪装
// ============================================================
class Mimic : public Enemy {
public:
    Mimic();
    virtual void UpdateAI(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Mimic"; }

    bool IsDisguised() const { return m_isDisguised; }
    void Reveal();  // 被触发后现形

private:
    bool    m_isDisguised;
    bool    m_hasTriggered;
    float   m_revealTimer;
};
