#pragma once

#include "GameObject.h"

// ============================================================
// Obstacle - 障碍物基类（不可移动的场景物体）
// ============================================================
class Obstacle : public GameObject {
public:
    Obstacle();
    virtual ~Obstacle() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Obstacle"; }

    ObstacleType GetObstacleType() const { return m_obstacleType; }
    void SetObstacleType(ObstacleType type) { m_obstacleType = type; }

    // 是否阻挡子弹
    bool BlocksBullets() const { return m_blocksBullets; }
    void SetBlocksBullets(bool v) { m_blocksBullets = v; }

    // 是否阻挡移动
    bool BlocksMovement() const { return m_blocksMovement; }
    void SetBlocksMovement(bool v) { m_blocksMovement = v; }

    // 是否有地面效果（玩家站在上面触发）
    virtual bool HasGroundEffect() const { return false; }
    virtual void ApplyGroundEffect(Player* player, float deltaTime) {}

    // 摩擦力系数（1.0 = 正常，< 1.0 = 滑行）
    float GetFrictionModifier() const { return m_frictionModifier; }
    void SetFrictionModifier(float f) { m_frictionModifier = f; }

protected:
    ObstacleType m_obstacleType;
    bool         m_blocksBullets;
    bool         m_blocksMovement;
    float        m_frictionModifier;   // 地面摩擦力修正
};

// ============================================================
// Tree - 树木 (Forest)，阻挡子弹和移动
// ============================================================
class Tree : public Obstacle {
public:
    Tree();
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Tree"; }
};

// ============================================================
// IcePatch - 冰块 (Ice Dungeon)，不阻挡，但减小摩擦力
// ============================================================
class IcePatch : public Obstacle {
public:
    IcePatch();
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "IcePatch"; }
    virtual bool HasGroundEffect() const override { return true; }
    virtual void ApplyGroundEffect(Player* player, float deltaTime) override;

    static float GetIceFriction() { return 0.15f; }  // 低摩擦力
};

// ============================================================
// IceBlock - 冰晶柱 (Ice Dungeon)，阻挡移动和子弹
// ============================================================
class IceBlock : public Obstacle {
public:
    IceBlock();
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "IceBlock"; }
};

// ============================================================
// LavaPool - 岩浆池 (Volcano)，不阻挡子弹，踩上扣血
// ============================================================
class LavaPool : public Obstacle {
public:
    LavaPool();
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "LavaPool"; }
    virtual bool HasGroundEffect() const override { return true; }
    virtual void ApplyGroundEffect(Player* player, float deltaTime) override;

    void SetLifetime(float seconds) { m_lifetime = seconds; }
    static int GetDamagePerSecond() { return 1; }

private:
    float m_lifetime;
    float m_age;
};
