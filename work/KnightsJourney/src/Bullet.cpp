// ============================================================
// Bullet.cpp - 子弹实体（直线飞行、寿命管理、EasyX 绘制）
// ============================================================

#include "../include/Entity/Bullet.h"
#include "../include/Graphics/VisualEffects.h"
#include <graphics.h>
#include <cmath>

namespace {

bool IsBeamWeapon(WeaponType type) {
    return type == WeaponType::SNIPER_RIFLE ||
           type == WeaponType::CRYSTAL_LASER ||
           type == WeaponType::THUNDER_RAIL ||
           type == WeaponType::SUNFIRE_SPEAR;
}

bool IsLargeOrbWeapon(WeaponType type) {
    return type == WeaponType::ROCKET_LAUNCHER ||
           type == WeaponType::ARCANE_ORB ||
           type == WeaponType::VOID_LAUNCHER ||
           type == WeaponType::METEOR_ROD;
}

} // namespace

// ============================================================
// 构造
// ============================================================
Bullet::Bullet()
    : m_faction(BulletFaction::PLAYER)
    , m_damage(1)
    , m_speed(400.0f)
    , m_inheritedForwardSpeed(0.0f)
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
    float travelSpeed = m_speed + m_inheritedForwardSpeed;
    m_position.x += m_direction.x * travelSpeed * deltaTime;
    m_position.y += m_direction.y * travelSpeed * deltaTime;

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

    int length = m_isMeleeSlash ? 24 : (m_explosive ? 18 : 14);
    int thick = m_isMeleeSlash ? 7 : (m_isCritical ? 6 : 4);
    COLORREF border = m_faction == BulletFaction::PLAYER ? RGB(33, 28, 12) : RGB(38, 12, 14);

    if (m_faction == BulletFaction::PLAYER && IsBeamWeapon(m_sourceWeapon)) {
        int length = m_speed <= 1.0f ? 66 : 34;
        int thick = m_sourceWeapon == WeaponType::THUNDER_RAIL ? 8 : 6;
        setlinestyle(PS_SOLID, thick + 4);
        setlinecolor(RGB(18, 35, 48));
        line(cx - (int)(m_direction.x * length / 2), cy - (int)(m_direction.y * length / 2),
             cx + (int)(m_direction.x * length / 2), cy + (int)(m_direction.y * length / 2));
        setlinestyle(PS_SOLID, thick);
        setlinecolor(bulletColor);
        line(cx - (int)(m_direction.x * length / 2), cy - (int)(m_direction.y * length / 2),
             cx + (int)(m_direction.x * length / 2), cy + (int)(m_direction.y * length / 2));
        setlinestyle(PS_SOLID, 2);
        setlinecolor(RGB(238, 255, 255));
        line(cx - (int)(m_direction.x * length / 3), cy - (int)(m_direction.y * length / 3),
             cx + (int)(m_direction.x * length / 3), cy + (int)(m_direction.y * length / 3));
        setlinestyle(PS_SOLID, 1);
    } else if (m_faction == BulletFaction::PLAYER && IsLargeOrbWeapon(m_sourceWeapon)) {
        int r = m_explosive ? (m_explosionRadius >= 120.0f ? 18 : 14) : 13;
        VisualFX::DrawGlowCircle(cx, cy, r + 8, bulletColor, 3);
        VisualFX::DrawPixelDiamond(cx, cy, r, bulletColor, RGB(35, 24, 45));
        setfillcolor(RGB(255, 236, 164));
        solidrectangle(cx - 4, cy - 4, cx + 4, cy + 4);
        setlinecolor(VisualFX::ScaleColor(bulletColor, 0.65f));
        line(cx - (int)(m_direction.x * (r + 12)), cy - (int)(m_direction.y * (r + 12)),
             cx - (int)(m_direction.x * 4), cy - (int)(m_direction.y * 4));
    } else if (m_explosive) {
        VisualFX::DrawPixelProjectile(cx, cy, m_direction.x, m_direction.y,
                                      length, thick, RGB(255, 122, 36), border);
        setfillcolor(RGB(255, 216, 76));
        solidrectangle(cx - 4, cy - 4, cx + 4, cy + 4);
    } else if (m_causesSlow) {
        VisualFX::DrawPixelProjectile(cx, cy, m_direction.x, m_direction.y,
                                      length, thick, RGB(126, 223, 248), RGB(18, 52, 72));
    } else if (m_causesBurning) {
        VisualFX::DrawPixelProjectile(cx, cy, m_direction.x, m_direction.y,
                                      length, thick, RGB(255, 83, 48), RGB(62, 19, 12));
    } else if (m_isMeleeSlash) {
        setlinestyle(PS_SOLID, 7);
        setlinecolor(RGB(16, 49, 55));
        line(cx - (int)(m_direction.y * 18), cy + (int)(m_direction.x * 18),
             cx + (int)(m_direction.y * 18), cy - (int)(m_direction.x * 18));
        setlinestyle(PS_SOLID, 4);
        setlinecolor(RGB(123, 242, 245));
        line(cx - (int)(m_direction.y * 15), cy + (int)(m_direction.x * 15),
             cx + (int)(m_direction.y * 15), cy - (int)(m_direction.x * 15));
        setlinestyle(PS_SOLID, 1);
    } else {
        VisualFX::DrawPixelProjectile(cx, cy, m_direction.x, m_direction.y,
                                      length, thick, bulletColor, border);
        if (m_isCritical) {
            setfillcolor(RGB(255, 245, 140));
            solidrectangle(cx - 2, cy - 2, cx + 2, cy + 2);
        }
    }
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
