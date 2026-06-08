#pragma once

#include <vector>
#include "Common.h"

// ============================================================
// PlayerBullet - 玩家子弹
// ============================================================
class PlayerBullet {
public:
    PlayerBullet(float x, float y, float vx, float vy, int damage, COLORREF color, float size);
    ~PlayerBullet() = default;

    void Update(float dt);

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetVX() const { return m_vx; }
    float GetVY() const { return m_vy; }
    float GetSize() const { return m_size; }
    int   GetDamage() const { return m_damage; }
    COLORREF GetColor() const { return m_color; }
    bool  IsAlive() const { return m_alive; }
    void  Kill() { m_alive = false; }

    // 拖尾
    struct TrailPoint { float x, y, life; };
    const std::vector<TrailPoint>& GetTrail() const { return m_trail; }

private:
    float m_x, m_y;
    float m_vx, m_vy;
    int   m_damage;
    COLORREF m_color;
    float m_size;
    bool  m_alive;
    float m_life;
    std::vector<TrailPoint> m_trail;
};

// ============================================================
// EnemyBullet - 敌人子弹
// ============================================================
class EnemyBullet {
public:
    EnemyBullet(float x, float y, float vx, float vy, COLORREF color, float size,
                BulletBehavior behavior = BulletBehavior::NONE);
    ~EnemyBullet() = default;

    void Update(float dt, int screenW, int screenH);

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetVX() const { return m_vx; }
    float GetVY() const { return m_vy; }
    float GetSize() const { return m_size; }
    COLORREF GetColor() const { return m_color; }
    bool  IsAlive() const { return m_alive; }
    void  Kill() { m_alive = false; }

    BulletBehavior GetBehavior() const { return m_behavior; }
    float GetTime() const { return m_time; }
    float GetLifeTimer() const { return m_lifeTimer; }
    float GetMaxLife() const { return m_maxLife; }

    int GetBounces() const { return m_bounces; }
    void SetBounces(int b) { m_bounces = b; }

    // laserOrb子子弹生成标志
    float GetBurstTimer() const { return m_burstTimer; }
    void  SetBurstTimer(float t) { m_burstTimer = t; }

private:
    float m_x, m_y;
    float m_vx, m_vy;
    COLORREF m_color;
    float m_size;
    bool  m_alive;
    float m_time;

    BulletBehavior m_behavior;
    int   m_bounces;
    float m_burstTimer;
    float m_burstInterval;
    float m_lifeTimer;
    float m_maxLife;
};
