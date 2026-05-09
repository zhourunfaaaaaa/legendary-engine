// ============================================================
// Bullet.cpp - 子弹实体（直线飞行、寿命管理、EasyX 绘制）
// ============================================================

#include "../include/Entity/Bullet.h"
#include <graphics.h>
#include <cmath>

// ============================================================
// 构造
// ============================================================
Bullet::Bullet()
    : m_faction(BulletFaction::PLAYER)
    , m_damage(1)
    , m_speed(400.0f)
    , m_direction(1.0f, 0.0f)
    , m_piercing(false)
    , m_homing(false)
    , m_bouncing(false)
    , m_explosive(false)
    , m_isMeleeSlash(false)
    , m_homingTarget(nullptr)
    , m_homingStrength(3.0f)
    , m_maxBounces(0)
    , m_bounceCount(0)
    , m_maxPierceCount(99)
    , m_pierceCount(0)
    , m_hitTargetCount(0)
    , m_explosionRadius(60.0f)
    , m_causesBurning(false)
    , m_burnChance(0.0f)
    , m_burnDuration(0.0f)
    , m_causesSlow(false)
    , m_slowDuration(0.0f)
    , m_hasLifeSteal(false)
    , m_lifeStealChance(0.0f)
    , m_lifetime(3.0f)
    , m_age(0.0f)
    , m_canBeBlocked(true)
    , m_sourceWeapon(WeaponType::ASSAULT_RIFLE)
    , m_isCritical(false)
    , m_color(0xFFFF00) {           // 默认黄色

    SetRenderLayer(8);              // 子弹渲染层级（玩家和敌人之间）
    SetCollisionSize(6.0f, 6.0f);   // 子弹碰撞盒半径 6
}

// ============================================================
// 设置方向（弧度角）
// ============================================================
void Bullet::SetDirection(float angleRad) {
    m_direction.x = std::cos(angleRad);
    m_direction.y = std::sin(angleRad);
}

// ============================================================
// 每帧更新
// ============================================================
void Bullet::Update(float deltaTime) {
    if (!m_active) return;

    // 1. 寿命递减
    m_age += deltaTime;
    if (m_age >= m_lifetime) {
        m_lifetime = 0.0f;
        MarkForDeletion();
        return;
    }

    // 2. 追踪逻辑（若启用）
    if (m_homing && m_homingTarget) {
        Vector2 targetPos = m_homingTarget->GetPosition();
        Vector2 toTarget = targetPos - m_position;
        float dist = toTarget.Length();
        if (dist > 0.1f) {
            Vector2 desiredDir = toTarget.Normalized();
            // 渐进转向
            float turnAmount = m_homingStrength * deltaTime;
            m_direction = (m_direction + desiredDir * turnAmount).Normalized();
        }
    }

    // 3. 直线移动
    m_position.x += m_direction.x * m_speed * deltaTime;
    m_position.y += m_direction.y * m_speed * deltaTime;

    // 4. 同步碰撞盒
    SyncAABBToPosition();

    // 5. 边界检查：飞出房间后销毁
    if (m_position.x < -50.0f || m_position.x > ROOM_WIDTH + 50.0f ||
        m_position.y < -50.0f || m_position.y > ROOM_HEIGHT + 50.0f) {
        MarkForDeletion();
    }
}

// ============================================================
// 渲染 - 黄色/白色小圆
// ============================================================
void Bullet::Render() {
    if (!m_active) return;

    int cx = static_cast<int>(m_position.x);
    int cy = static_cast<int>(m_position.y);

    // 根据子弹归属和类型选择颜色
    COLORREF bulletColor;
    if (m_isMeleeSlash) {
        bulletColor = RGB(0, 255, 255);       // 刀光：青色
    } else if (m_faction == BulletFaction::PLAYER) {
        bulletColor = m_color;                 // 玩家子弹：默认黄色
    } else {
        // 敌方子弹根据类型区分
        if (m_explosive) {
            bulletColor = RGB(255, 100, 30);   // 爆炸弹：橙红
        } else if (m_causesBurning) {
            bulletColor = RGB(255, 60, 60);    // 火焰弹：红色
        } else if (m_causesSlow) {
            bulletColor = RGB(100, 200, 255);  // 冰冻弹：浅蓝
        } else {
            bulletColor = RGB(255, 80, 80);    // 普通敌方子弹：红色
        }
    }

    int radius = m_isMeleeSlash ? 5 : 3;

    // 绘制小圆（带外圈发光效果）
    // 外圈光晕
    setfillcolor(RGB(
        (GetRValue(bulletColor) >> 1),
        (GetGValue(bulletColor) >> 1),
        (GetBValue(bulletColor) >> 1)
    ));
    solidcircle(cx, cy, radius + 1);

    // 主体
    setfillcolor(bulletColor);
    solidcircle(cx, cy, radius);

    // 高光点
    setfillcolor(RGB(255, 255, 255));
    solidcircle(cx - 1, cy - 1, 1);
}

// ============================================================
// ExplosionEffect - 爆炸视觉特效
// ============================================================
ExplosionEffect::ExplosionEffect()
    : m_maxRadius(60.0f), m_currentRadius(0.0f), m_lifetime(0.35f), m_age(0.0f) {
    SetRenderLayer(5);
}

void ExplosionEffect::Update(float deltaTime) {
    m_age += deltaTime;
    if (m_age >= m_lifetime) {
        MarkForDeletion();
        return;
    }
    float progress = m_age / m_lifetime;
    m_currentRadius = m_maxRadius * progress;  // 从小扩散到大
}

void ExplosionEffect::Render() {
    float progress = m_age / m_lifetime;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    int r = (int)m_currentRadius;

    // 外圈（橙色，逐渐透明 = 颜色变暗）
    int alpha = (int)(255 * (1.0f - progress));
    COLORREF outerColor = RGB(
        (255 * alpha) / 255,
        (140 * alpha) / 255,
        (20 * alpha) / 255
    );
    setfillcolor(outerColor);
    solidcircle(cx, cy, r);

    // 内圈（亮黄色）
    int innerR = r * 2 / 3;
    COLORREF innerColor = RGB(
        (255 * alpha) / 255,
        (220 * alpha) / 255,
        (60 * alpha) / 255
    );
    setfillcolor(innerColor);
    solidcircle(cx, cy, innerR);

    // 中心白光
    if (r > 4) {
        int coreR = r / 4;
        setfillcolor(RGB(255, 255, 200));
        solidcircle(cx, cy, coreR);
    }
}
