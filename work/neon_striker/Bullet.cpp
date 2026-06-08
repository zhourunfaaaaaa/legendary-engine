#include "Bullet.h"
#include "GameManager.h"

// ============================================================
// PlayerBullet
// ============================================================
PlayerBullet::PlayerBullet(float x, float y, float vx, float vy,
                           int damage, COLORREF color, float size)
    : m_x(x), m_y(y), m_vx(vx), m_vy(vy)
    , m_damage(damage), m_color(color), m_size(size)
    , m_alive(true), m_life(0.0f)
{
}

void PlayerBullet::Update(float dt) {
    m_life += dt;
    m_trail.push_back({ m_x, m_y, 0.1f });
    for (int i = (int)m_trail.size() - 1; i >= 0; i--) {
        m_trail[i].life -= dt;
        if (m_trail[i].life <= 0.0f) {
            m_trail.erase(m_trail.begin() + i);
        }
    }

    m_x += m_vx * dt;
    m_y += m_vy * dt;

    if (m_y < -50 || m_y > WINDOW_HEIGHT + 50 ||
        m_x < -50 || m_x > WINDOW_WIDTH + 50) {
        m_alive = false;
    }
}

// ============================================================
// EnemyBullet
// ============================================================
EnemyBullet::EnemyBullet(float x, float y, float vx, float vy,
                         COLORREF color, float size,
                         BulletBehavior behavior)
    : m_x(x), m_y(y), m_vx(vx), m_vy(vy)
    , m_color(color), m_size(size)
    , m_alive(true), m_time(0.0f)
    , m_behavior(behavior)
    , m_bounces(behavior == BulletBehavior::BOUNCE ? 3 : 0)
    , m_burstTimer(0.15f), m_burstInterval(0.15f)
    , m_lifeTimer(0.0f), m_maxLife(4.0f)
{
}

void EnemyBullet::Update(float dt, int screenW, int screenH) {
    m_time += dt;

    // bounce 行为
    if (m_behavior == BulletBehavior::BOUNCE && m_bounces > 0) {
        if (m_x < 10 || m_x > screenW - 10) {
            m_vx = -m_vx;
            m_bounces--;
            m_x = ClampFloat(m_x, 10.0f, (float)(screenW - 10));
        }
        if (m_y < 10 || m_y > screenH - 10) {
            m_vy = -m_vy;
            m_bounces--;
            m_y = ClampFloat(m_y, 10.0f, (float)(screenH - 10));
        }
    }

    // laserOrb 行为
    if (m_behavior == BulletBehavior::LASER_ORB) {
        m_lifeTimer += dt;
        m_burstTimer -= dt;
    }

    m_x += m_vx * dt;
    m_y += m_vy * dt;

    // laserOrb 超时死亡
    if (m_behavior == BulletBehavior::LASER_ORB && m_lifeTimer > m_maxLife) {
        m_alive = false;
    }
    if (m_y > screenH + 30 || m_y < -30 ||
        m_x > screenW + 30 || m_x < -30) {
        m_alive = false;
    }
}
