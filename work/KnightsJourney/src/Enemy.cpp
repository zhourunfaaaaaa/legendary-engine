// ============================================================
// Enemy.cpp - 怪物基类 + 哥布林近战实现
// ============================================================

#include "../include/Entity/Enemy.h"
#include "../include/Entity/Player.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/DropItem.h"
#include "../include/Entity/Obstacle.h"
#include "../include/System/EntityManager.h"
#include "../include/System/BuffManager.h"
#include "../include/Core/GameManager.h"
#include "../include/Graphics/VisualEffects.h"
#include <graphics.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

Vector2 Perpendicular(const Vector2& v) {
    return Vector2(-v.y, v.x);
}

Vector2 ClampRoomVector(const Vector2& pos, const Vector2& desired) {
    Vector2 dir = desired;
    const float edge = ROOM_ENTITY_MARGIN + 12.0f;
    if (pos.x < edge && dir.x < 0.0f) dir.x *= -0.45f;
    if (pos.x > ROOM_WIDTH - edge && dir.x > 0.0f) dir.x *= -0.45f;
    if (pos.y < edge && dir.y < 0.0f) dir.y *= -0.45f;
    if (pos.y > ROOM_HEIGHT - edge && dir.y > 0.0f) dir.y *= -0.45f;
    return dir.Normalized();
}

void DrawEnemyShadow(int cx, int cy, int radius) {
    setfillcolor(RGB(8, 11, 13));
    solidcircle(cx + 2, cy + radius / 2 + 4, radius + 3);
}

void DrawEnemyHP(int cx, int cy, int radius, int hp, int maxHP) {
    if (hp >= maxHP || maxHP <= 0) return;
    int barW = radius * 2 + 8;
    int barH = 5;
    int barX = cx - barW / 2;
    int barY = cy - radius - 13;
    float hpRatio = (float)hp / maxHP;
    VisualFX::DrawProgressBar(barX, barY, barW, barH, hpRatio,
                              hpRatio > 0.5f ? RGB(72, 218, 96) :
                              hpRatio > 0.25f ? RGB(229, 185, 58) : RGB(231, 68, 72),
                              RGB(35, 35, 35), RGB(80, 83, 86));
}

void DrawPixelHumanoid(int cx, int cy, int w, int h,
                       COLORREF body, COLORREF outline, COLORREF accent) {
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 8);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2 + 6, cx + w / 2, cy + h / 2,
                            body, outline, 5);
    VisualFX::DrawPixelRect(cx - w / 2 + 7, cy - h / 2 - 10,
                            cx + w / 2 - 7, cy - h / 2 + 14,
                            VisualFX::ScaleColor(body, 1.12f), outline, 5);
    setfillcolor(accent);
    solidrectangle(cx - 9, cy - h / 2 - 1, cx - 4, cy - h / 2 + 5);
    solidrectangle(cx + 4, cy - h / 2 - 1, cx + 9, cy - h / 2 + 5);
    setfillcolor(RGB(14, 15, 18));
    solidrectangle(cx - 7, cy - h / 2 + 1, cx - 5, cy - h / 2 + 3);
    solidrectangle(cx + 6, cy - h / 2 + 1, cx + 8, cy - h / 2 + 3);
}

void DrawPixelBeast(int cx, int cy, int w, int h,
                    COLORREF body, COLORREF outline, COLORREF accent) {
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 9);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2,
                            body, outline, 5);
    setfillcolor(VisualFX::ScaleColor(body, 1.18f));
    solidrectangle(cx - w / 2 + 5, cy - h / 2 - 8, cx - w / 2 + 16, cy - h / 2 + 5);
    solidrectangle(cx + w / 2 - 16, cy - h / 2 - 8, cx + w / 2 - 5, cy - h / 2 + 5);
    setfillcolor(accent);
    solidrectangle(cx - 10, cy - 5, cx - 4, cy + 1);
    solidrectangle(cx + 4, cy - 5, cx + 10, cy + 1);
}

void TrySpawnPotionDrop(const Vector2& position, EntityManager& entityMgr) {
    int level = GameManager::GetInstance().GetCurrentLevel();
    float potionChance = 0.30f;
    float hpChance = 0.15f;
    int mpValue = 50;

    if (level >= 3) {
        potionChance = 0.70f;
        hpChance = 0.08f;
        mpValue = 80;
    } else if (level >= 2) {
        potionChance = 0.50f;
        hpChance = 0.10f;
        mpValue = 65;
    }

    if (RandomFloat(0.0f, 1.0f) >= potionChance) return;

    DropItem* potion = entityMgr.SpawnEntity<DropItem>();
    potion->SetPosition(position.x + RandomFloat(-22.0f, 22.0f),
                        position.y + RandomFloat(-22.0f, 22.0f));
    bool isHP = RandomFloat(0.0f, 1.0f) < hpChance;
    potion->SetDropType(isHP ? DropType::HP_POTION : DropType::MP_POTION);
    potion->SetValue(isHP ? 2 : mpValue);
}

} // namespace

// ============================================================
// Enemy 基类 构造
// ============================================================
Enemy::Enemy()
    : m_aiState(EnemyState::IDLE)
    , m_pTarget(nullptr)
    , m_damage(2)
    , m_attackRange(30.0f)
    , m_attackCooldown(1.0f)
    , m_attackTimer(0.0f)
    , m_detectionRange(350.0f)
    , m_patrolOrigin(0.0f, 0.0f)
    , m_patrolRadius(60.0f)
    , m_patrolAngle(0.0f)
    , m_patrolSpeed(40.0f)
    , m_goldDrop(3)
    , m_expDrop(5)
    , m_isElite(false)
    , m_eliteScale(1.5f)
    , m_biome(BiomeType::FOREST)
    , m_damageFlashTimer(0.0f)
    , m_idleWanderTimer(0.0f)
    , m_idleWanderDir(0.0f, 0.0f)
    , m_aiDecisionTimer(0.0f)
    , m_tacticalDir(1.0f, 0.0f) {

    SetRenderLayer(6);
    SetCollisionSize(14.0f, 14.0f);

    SetMaxHP(5);
    SetHP(5);
    SetMoveSpeed(120.0f);
}

// ============================================================
// 每帧更新
// ============================================================
void Enemy::Update(float deltaTime) {
    if (m_isDead) return;

    UpdateStatusEffects(deltaTime);

    // 受伤闪烁倒计时
    if (m_damageFlashTimer > 0.0f) {
        m_damageFlashTimer -= deltaTime;
    }

    // 攻击冷却
    if (m_attackTimer > 0.0f) {
        m_attackTimer -= deltaTime;
    }
    if (m_aiDecisionTimer > 0.0f) {
        m_aiDecisionTimer -= deltaTime;
    }

    // AI
    if (!m_isStunned && m_pTarget) {
        UpdateAI(deltaTime);
    }

    // 闲逛状态：远距离无规律慢速移动
    if (!m_isStunned && m_aiState == EnemyState::IDLE) {
        m_idleWanderTimer -= deltaTime;
        if (m_idleWanderTimer <= 0.0f) {
            m_idleWanderTimer = RandomFloat(0.8f, 2.0f);
            float angle = RandomFloat(0.0f, 2.0f * PI);
            m_idleWanderDir = Vector2(std::cos(angle), std::sin(angle));
        }
        m_position += m_idleWanderDir * m_moveSpeed * 0.35f * deltaTime;
    }

    // 移动
    if (!m_isStunned && (m_aiState == EnemyState::CHASE || m_aiState == EnemyState::PATROL)) {
        m_position += m_moveDirection * m_moveSpeed * deltaTime;
    }

    // 边界限制
    const float margin = ROOM_ENTITY_MARGIN;
    if (m_position.x < margin) m_position.x = margin;
    if (m_position.y < margin) m_position.y = margin;
    if (m_position.x > ROOM_WIDTH  - margin) m_position.x = ROOM_WIDTH  - margin;
    if (m_position.y > ROOM_HEIGHT - margin) m_position.y = ROOM_HEIGHT - margin;

    SyncAABBToPosition();
}

// ============================================================
// 渲染（基类默认：灰色圆，派生类可覆写）
// ============================================================
void Enemy::Render() {
    if (m_isDead) return;

    int cx = static_cast<int>(m_position.x);
    int cy = static_cast<int>(m_position.y);
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int radius = static_cast<int>(14.0f * scale);

    DrawEnemyShadow(cx, cy, radius);
    if (m_isElite) VisualFX::DrawGlowCircle(cx, cy, radius, RGB(235, 154, 45), 3);

    // 受伤闪烁：红色
    bool flashRed = (m_damageFlashTimer > 0.0f);
    int flashPhase = static_cast<int>(m_damageFlashTimer * 30) % 2;

    // 描边
    setfillcolor(flashRed ? RGB(60, 0, 0) : RGB(30, 30, 30));
    solidcircle(cx, cy, radius + 2);

    // 主体
    COLORREF bodyColor;
    if (flashRed && flashPhase == 1) {
        bodyColor = RGB(255, 60, 60);
    } else {
        bodyColor = RGB(140, 140, 150);
    }
    setfillcolor(bodyColor);
    solidcircle(cx, cy, radius);

    // 尖耳与短刃，强化怪物轮廓
    setfillcolor(VisualFX::ScaleColor(bodyColor, 0.7f));
    solidcircle(cx - radius + 2, cy - 4, 5);
    solidcircle(cx + radius - 2, cy - 4, 5);
    setlinecolor(RGB(225, 225, 210));
    setlinestyle(PS_SOLID, 2);
    line(cx + radius - 2, cy + 2, cx + radius + 11, cy + 9);
    setlinestyle(PS_SOLID, 1);

    // HP 条（仅受伤时显示）
    if (m_hp < m_maxHP) {
        int barW = radius * 2 + 4;
        int barH = 4;
        int barX = cx - barW / 2;
        int barY = cy - radius - 10;
        float hpRatio = (float)m_hp / m_maxHP;

        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpRatio > 0.5f ? RGB(60, 200, 60) :
                     hpRatio > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + static_cast<int>(barW * hpRatio), barY + barH);
    }
}

// ============================================================
// 受击
// ============================================================
void Enemy::TakeDamage(int damage) {
    if (m_isDead) return;
    Character::TakeDamage(damage);
    m_damageFlashTimer = 0.12f;
}

void Enemy::Die() {
    m_isDead = true;
    m_aiState = EnemyState::DEAD;

    // 嗜血天赋：击杀回复 1 HP
    BuffManager& buffs = GameManager::GetInstance().GetBuffManager();
    if (buffs.HasBloodthirst()) {
        Player* player = GameManager::GetInstance().GetPlayer();
        if (player && !player->IsDead()) {
            player->Heal(1);
        }
    }

    // 掉落金币
    EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
    DropItem* gold = entityMgr.SpawnEntity<DropItem>();
    gold->SetPosition(m_position);
    gold->SetDropType(DropType::GOLD);
    gold->SetValue(m_goldDrop);

    // 30% 额外掉落药水（MP为主，HP极少）
    if (RandomFloat(0.0f, 1.0f) < 0.30f) {
        DropItem* potion = entityMgr.SpawnEntity<DropItem>();
        potion->SetPosition(m_position.x + RandomFloat(-20.0f, 20.0f),
                            m_position.y + RandomFloat(-20.0f, 20.0f));
        bool isHP = RandomFloat(0.0f, 1.0f) < 0.15f;  // 85% MP, 15% HP
        potion->SetDropType(isHP ? DropType::HP_POTION : DropType::MP_POTION);
        potion->SetValue(isHP ? 2 : 50);  // HP 回复 2，MP 回复 50
    }

    if (GameManager::GetInstance().GetCurrentLevel() >= 2) {
        TrySpawnPotionDrop(m_position, entityMgr);
    }

    MarkForDeletion();
}

// ============================================================
// AI 状态机
// ============================================================
void Enemy::SetAIState(EnemyState state) {
    if (m_aiState == state) return;
    m_aiState = state;
}

void Enemy::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();

    switch (m_aiState) {
        case EnemyState::IDLE:
            if (dist < m_detectionRange) {
                SetAIState(EnemyState::CHASE);
            }
            break;

        case EnemyState::PATROL:
            UpdatePatrol(deltaTime);
            if (dist < m_detectionRange) {
                SetAIState(EnemyState::CHASE);
            }
            break;

        case EnemyState::CHASE:
            UpdateChase(deltaTime);
            if (dist < m_attackRange) {
                SetAIState(EnemyState::ATTACK);
            }
            break;

        case EnemyState::ATTACK:
            UpdateAttack(deltaTime);
            if (dist > m_attackRange * 1.3f) {
                SetAIState(EnemyState::CHASE);
            }
            break;

        case EnemyState::STUNNED:
            break;

        case EnemyState::DEAD:
            break;
    }
}

void Enemy::UpdatePatrol(float deltaTime) {
    m_patrolAngle += m_patrolSpeed * deltaTime;
    float px = m_patrolOrigin.x + std::cos(m_patrolAngle) * m_patrolRadius;
    float py = m_patrolOrigin.y + std::sin(m_patrolAngle) * m_patrolRadius;
    m_moveDirection = (Vector2(px, py) - m_position).Normalized();
}

void Enemy::UpdateChase(float deltaTime) {
    m_moveDirection = GetDirectionToTarget();
}

void Enemy::UpdateAttack(float deltaTime) {
    // 面向目标
    m_moveDirection = GetDirectionToTarget();
    // 攻击
    Attack();
}

void Enemy::Attack() {
    if (!CanAttack()) return;
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();
    if (dist > m_attackRange) return;

    m_pTarget->TakeDamage(m_damage);
    m_attackTimer = m_attackCooldown;
}

void Enemy::FireBullet(float speed, unsigned int color, int bulletDamage) {
    if (!CanAttack()) return;
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();
    if (dist > m_attackRange) return;

    EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
    Bullet* bullet = entityMgr.SpawnEntity<Bullet>();
    bullet->SetPosition(m_position);
    bullet->SetDirection(GetDirectionToTarget());
    bullet->SetSpeed(speed);
    bullet->SetDamage(bulletDamage >= 0 ? bulletDamage : m_damage);
    bullet->SetFaction(BulletFaction::ENEMY);
    bullet->SetColor(color);
    bullet->SetCollisionSize(6.0f, 6.0f);

    m_attackTimer = m_attackCooldown;
}

// ============================================================
// 目标追踪工具函数
// ============================================================
float Enemy::GetDistanceToTarget() const {
    if (!m_pTarget) return 99999.0f;
    return Vector2::Distance(m_position, m_pTarget->GetPosition());
}

Vector2 Enemy::GetDirectionToTarget() const {
    if (!m_pTarget) return Vector2(1.0f, 0.0f);
    Vector2 dir = m_pTarget->GetPosition() - m_position;
    float len = dir.Length();
    if (len < 0.001f) return Vector2(1.0f, 0.0f);
    return dir * (1.0f / len);
}

bool Enemy::IsTargetInRange(float range) const {
    return GetDistanceToTarget() <= range;
}

// ============================================================
// 精英设置
// ============================================================
void Enemy::SetElite(bool elite) {
    m_isElite = elite;
    if (elite) {
        m_maxHP = static_cast<int>(m_maxHP * 1.8f);
        m_hp = m_maxHP;
        m_damage = static_cast<int>(m_damage * 1.5f);
        m_goldDrop *= 2;
        m_moveSpeed *= 1.2f;
        SetCollisionSize(14.0f * m_eliteScale, 14.0f * m_eliteScale);
    }
}

// ============================================================
// 哥布林近战 (GoblinMelee)
// ============================================================
GoblinMelee::GoblinMelee() {
    m_damage         = 2;
    m_attackRange    = 32.0f;
    m_attackCooldown = 0.8f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 3;
    m_moveSpeed      = 100.0f;
    m_biome          = BiomeType::FOREST;

    SetMaxHP(5);
    SetHP(5);
    SetRenderLayer(6);

    SetAIState(EnemyState::IDLE);
}

void GoblinMelee::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();

    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        Vector2 toTarget = GetDirectionToTarget();
        if (m_aiDecisionTimer <= 0.0f) {
            m_aiDecisionTimer = RandomFloat(0.45f, 0.85f);
            float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
            m_tacticalDir = (toTarget + Perpendicular(toTarget) * side * RandomFloat(0.28f, 0.55f)).Normalized();
        }
        m_moveDirection = ClampRoomVector(m_position, m_tacticalDir);
        if (dist < m_attackRange) {
            m_moveDirection = Vector2(0.0f, 0.0f);
            Attack();
        }
    } else {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
    }
}

void GoblinMelee::Render() {
    if (m_isDead) return;

    int cx = static_cast<int>(m_position.x);
    int cy = static_cast<int>(m_position.y);
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int w = m_isElite ? 42 : 34;
    int h = m_isElite ? 42 : 34;

    bool flashRed = (m_damageFlashTimer > 0.0f);
    int flashPhase = static_cast<int>(m_damageFlashTimer * 30) % 2;

    COLORREF bodyColor = (flashRed && flashPhase == 1) ? RGB(238, 65, 65) :
                         (m_isElite ? RGB(214, 132, 38) : RGB(78, 174, 74));
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 8);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2 + 4, cx + w / 2, cy + h / 2,
                            bodyColor, RGB(15, 34, 19), 5);
    VisualFX::DrawPixelRect(cx - w / 2 + 6, cy - h / 2 - 10,
                            cx + w / 2 - 6, cy - h / 2 + 13,
                            VisualFX::ScaleColor(bodyColor, 1.12f), RGB(15, 34, 19), 5);
    setfillcolor(VisualFX::ScaleColor(bodyColor, 0.85f));
    solidrectangle(cx - w / 2 - 8, cy - h / 2 - 2, cx - w / 2 + 4, cy - h / 2 + 8);
    solidrectangle(cx + w / 2 - 4, cy - h / 2 - 2, cx + w / 2 + 8, cy - h / 2 + 8);

    if (!(flashRed && flashPhase == 1)) {
        setfillcolor(RGB(255, 244, 220));
        solidrectangle(cx - 9, cy - h / 2 - 2, cx - 4, cy - h / 2 + 4);
        solidrectangle(cx + 4, cy - h / 2 - 2, cx + 9, cy - h / 2 + 4);
        setfillcolor(RGB(16, 18, 20));
        solidrectangle(cx - 7, cy - h / 2, cx - 5, cy - h / 2 + 2);
        solidrectangle(cx + 6, cy - h / 2, cx + 8, cy - h / 2 + 2);
    }

    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
}

// ============================================================
// GoblinArcher - 哥布林弓箭手 (Forest)
// ============================================================
GoblinArcher::GoblinArcher()
    : m_preferredDistance(250.0f) {
    m_damage         = 2;
    m_attackRange    = 430.0f;
    m_attackCooldown = 1.15f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 4;
    m_moveSpeed      = 115.0f;
    m_biome          = BiomeType::FOREST;
    SetMaxHP(4);
    SetHP(4);
    SetAIState(EnemyState::IDLE);
}

void GoblinArcher::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        Vector2 toTarget = GetDirectionToTarget();
        if (m_aiDecisionTimer <= 0.0f) {
            m_aiDecisionTimer = RandomFloat(0.7f, 1.15f);
            float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
            m_tacticalDir = Perpendicular(toTarget) * side;
        }
        if (dist < m_preferredDistance * 0.6f) {
            m_moveDirection = ClampRoomVector(m_position, (m_position - m_pTarget->GetPosition()).Normalized() + m_tacticalDir * 0.25f);
        } else if (dist > m_preferredDistance * 1.2f) {
            m_moveDirection = ClampRoomVector(m_position, toTarget + m_tacticalDir * 0.2f);
        } else {
            m_moveDirection = ClampRoomVector(m_position, m_tacticalDir * 0.95f);
        }
        if (dist < m_attackRange) {
            FireBullet(260.0f, 0xCC8844, 2);  // 棕色箭矢，2点伤害
        }
    } else {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
    }
}

void GoblinArcher::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(12.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    COLORREF body = (flash && fp == 1) ? RGB(238, 65, 65) : RGB(167, 132, 58);
    int w = m_isElite ? 40 : 32;
    int h = m_isElite ? 40 : 32;
    DrawPixelHumanoid(cx, cy, w, h, body, RGB(44, 31, 15), RGB(255, 244, 220));
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    if (m_isElite) VisualFX::DrawGlowCircle(cx, cy, r, RGB(225, 154, 64), 3);
    setfillcolor(flash ? RGB(60, 10, 10) : RGB(40, 30, 10));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(160, 130, 60));
    solidcircle(cx, cy, r);
    setlinecolor(RGB(112, 72, 28));
    setlinestyle(PS_SOLID, 3);
    line(cx - r - 9, cy + 7, cx + r + 9, cy - 7);
    setlinecolor(RGB(230, 202, 120));
    setlinestyle(PS_SOLID, 2);
    line(cx - r - 5, cy + 5, cx + r + 5, cy - 5);
    setlinestyle(PS_SOLID, 1);
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(255, 255, 255));
        solidcircle(cx + 3, cy - 4, 2);
        solidcircle(cx - 3, cy - 4, 2);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

// ============================================================
// ChargeBoar - 冲锋野猪 (Forest)
// ============================================================
ChargeBoar::ChargeBoar()
    : m_boarState(BoarState::IDLE), m_chargeTimer(0.0f), m_rushTimer(0.0f)
    , m_stunTimer(0.0f), m_rushSpeed(330.0f) {
    m_damage         = 2;
    m_attackRange    = 40.0f;
    m_attackCooldown = 1.0f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 112.0f;
    m_biome          = BiomeType::FOREST;
    SetMaxHP(8);
    SetHP(8);
    SetAIState(EnemyState::IDLE);
}

void ChargeBoar::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    switch (m_boarState) {
        case BoarState::IDLE:
            if (dist < m_detectionRange) {
                SetAIState(EnemyState::CHASE);
                m_boarState = BoarState::CHARGING;
                m_chargeTimer = 0.35f;
                m_rushDirection = GetDirectionToTarget();
            }
            break;
        case BoarState::CHARGING:
            m_chargeTimer -= deltaTime;
            m_moveDirection = ClampRoomVector(m_position, GetDirectionToTarget() * 0.55f);
            m_rushDirection = (m_rushDirection * 0.82f + GetDirectionToTarget() * 0.18f).Normalized();
            if (m_chargeTimer <= 0.0f) { m_boarState = BoarState::RUSHING; m_rushTimer = 0.62f; }
            break;
        case BoarState::RUSHING:
            m_rushTimer -= deltaTime;
            m_moveDirection = Vector2(0.0f, 0.0f);
            m_position += m_rushDirection * m_rushSpeed * deltaTime;
            if (dist < m_attackRange + 8.0f) Attack();
            if (m_rushTimer <= 0.0f) {
                m_boarState = BoarState::STUNNED_AFTER_WALL;
                m_stunTimer = 0.36f;
                float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
                m_tacticalDir = (GetDirectionToTarget() + Perpendicular(GetDirectionToTarget()) * side * 0.55f).Normalized();
            }
            break;
        case BoarState::STUNNED_AFTER_WALL:
            m_stunTimer -= deltaTime;
            SetAIState(EnemyState::CHASE);
            m_moveDirection = ClampRoomVector(m_position, m_tacticalDir);
            if (dist < m_attackRange) Attack();
            if (m_stunTimer <= 0.0f) {
                m_boarState = BoarState::CHARGING;
                m_chargeTimer = 0.28f;
                m_rushDirection = GetDirectionToTarget();
            }
            break;
    }
}

void ChargeBoar::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(16.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 58 : 46;
    int h = m_isElite ? 44 : 34;
    COLORREF body = (flash && fp == 1) ? RGB(238, 65, 65) : RGB(151, 77, 36);
    DrawPixelBeast(cx, cy, w, h, body, RGB(41, 23, 14), RGB(247, 231, 195));
    setfillcolor(RGB(240, 226, 188));
    solidrectangle(cx - w / 2 - 5, cy + 1, cx - w / 2 + 7, cy + 7);
    solidrectangle(cx + w / 2 - 7, cy + 1, cx + w / 2 + 5, cy + 7);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    setfillcolor(flash ? RGB(50, 10, 5) : RGB(40, 20, 5));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(140, 60, 20));
    solidcircle(cx, cy, r);
    setfillcolor(RGB(235, 222, 185));
    solidcircle(cx - r / 2, cy + 4, 3);
    solidcircle(cx + r / 2, cy + 4, 3);
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(255, 255, 255));
        solidcircle(cx - 6, cy - 6, 3);
        solidcircle(cx + 6, cy - 6, 3);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

// ============================================================
// ForestWisp - 森林幽灯 (Forest)
// ============================================================
ForestWisp::ForestWisp()
    : m_orbitAngle(RandomFloat(0.0f, 2.0f * PI))
    , m_preferredDistance(245.0f) {
    m_damage         = 1;
    m_attackRange    = 470.0f;
    m_attackCooldown = 1.05f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 4;
    m_moveSpeed      = 138.0f;
    m_biome          = BiomeType::FOREST;
    SetMaxHP(5);
    SetHP(5);
    SetCollisionSize(12.0f, 12.0f);
    SetAIState(EnemyState::IDLE);
}

void ForestWisp::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();
    if (dist >= m_detectionRange) {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
        return;
    }

    SetAIState(EnemyState::CHASE);
    Vector2 toTarget = GetDirectionToTarget();
    m_orbitAngle += deltaTime * 2.1f;
    float orbitSide = std::sin(m_orbitAngle) >= 0.0f ? 1.0f : -1.0f;
    Vector2 orbit = Perpendicular(toTarget) * orbitSide;

    if (dist < m_preferredDistance * 0.72f) {
        m_moveDirection = ClampRoomVector(m_position,
            (m_position - m_pTarget->GetPosition()).Normalized() + orbit * 0.55f);
    } else if (dist > m_preferredDistance * 1.25f) {
        m_moveDirection = ClampRoomVector(m_position, toTarget * 0.65f + orbit * 0.45f);
    } else {
        m_moveDirection = ClampRoomVector(m_position, orbit * 0.95f + toTarget * 0.12f);
    }

    if (dist < m_attackRange && CanAttack()) {
        EntityManager& em = GameManager::GetInstance().GetEntityManager();
        Vector2 lead = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 36.0f;
        Vector2 aim = (lead - m_position).Normalized();
        float baseAngle = std::atan2(aim.y, aim.x);
        for (int i = 0; i < 3; ++i) {
            float offset = (i - 1) * DegToRad(9.0f);
            Bullet* b = em.SpawnEntity<Bullet>();
            b->SetPosition(m_position.x + aim.x * 12.0f, m_position.y + aim.y * 12.0f);
            b->SetDirection(baseAngle + offset);
            b->SetSpeed(230.0f + i * 12.0f);
            b->SetDamage(1);
            b->SetLifetime(3.4f);
            b->SetFaction(BulletFaction::ENEMY);
            b->SetColor(0x8DFF74);
            b->SetCollisionSize(5.0f, 5.0f);
        }
        m_attackTimer = m_attackCooldown;
    }
}

void ForestWisp::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int size = m_isElite ? 25 : 19;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) :
                    (m_isElite ? RGB(169, 234, 79) : RGB(113, 224, 110));
    VisualFX::DrawPixelShadow(cx, cy + 14, 18, 6);
    VisualFX::DrawGlowCircle(cx, cy, size + 8, RGB(117, 224, 101), 3);
    VisualFX::DrawPixelDiamond(cx, cy, size, body, RGB(17, 43, 22));
    VisualFX::DrawPixelDiamond(cx, cy - 2, size / 2, RGB(211, 255, 164), RGB(33, 88, 39));
    setfillcolor(RGB(12, 21, 16));
    solidrectangle(cx - 5, cy - 2, cx - 3, cy);
    solidrectangle(cx + 3, cy - 2, cx + 5, cy);
    DrawEnemyHP(cx, cy, size, m_hp, m_maxHP);
}

// ============================================================
// IceSlime - 冰原史莱姆 (Ice Dungeon)
// ============================================================
IceSlime::IceSlime() : m_isSmall(false) {
    m_damage         = 2;
    m_attackRange    = 30.0f;
    m_attackCooldown = 1.2f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 2;
    m_moveSpeed      = 74.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(6);
    SetHP(6);
    SetAIState(EnemyState::CHASE);
}

void IceSlime::SetAsSmallSlime() {
    m_isSmall = true;
    SetMaxHP(2);
    SetHP(2);
    m_damage = 2;
    SetCollisionSize(8.0f, 8.0f);
}

void IceSlime::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        if (m_aiDecisionTimer <= 0.0f) {
            m_aiDecisionTimer = m_isSmall ? RandomFloat(0.35f, 0.65f) : RandomFloat(0.7f, 1.05f);
            Vector2 toTarget = GetDirectionToTarget();
            float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
            m_tacticalDir = (toTarget + Perpendicular(toTarget) * side * RandomFloat(0.1f, 0.35f)).Normalized();
        }
        m_moveDirection = ClampRoomVector(m_position, m_tacticalDir);
        if (dist < m_attackRange) Attack();
    } else {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
    }
}

void IceSlime::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    int r = m_isSmall ? 8 : (int)(14.0f * (m_isElite ? m_eliteScale : 1.0f));
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isSmall ? 22 : 36;
    int h = m_isSmall ? 20 : 30;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(82, 188, 222);
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 6);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2,
                            body, RGB(20, 49, 65), 4);
    setfillcolor(RGB(174, 235, 247));
    solidrectangle(cx - w / 3, cy - h / 3, cx + 2, cy - h / 3 + 5);
    setfillcolor(RGB(18, 28, 34));
    solidrectangle(cx - 7, cy + 1, cx - 4, cy + 4);
    solidrectangle(cx + 4, cy + 1, cx + 7, cy + 4);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    VisualFX::DrawGlowCircle(cx, cy, r, RGB(87, 194, 229), m_isSmall ? 2 : 3);
    setfillcolor(RGB(20, 40, 60));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 80, 80) : RGB(80, 180, 220));
    solidcircle(cx, cy, r);
    setfillcolor(RGB(151, 225, 245));
    solidcircle(cx - r / 4, cy - r / 3, (std::max)(2, r / 4));
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(200, 240, 255));
        solidcircle(cx - 3, cy - 3, 3);
        solidcircle(cx + 3, cy - 3, 3);
    }
    // HP 条
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

void IceSlime::Die() {
    if (m_isDead) return;
    m_isDead = true;

    if (m_isSmall) {
        // 小史莱姆直接死亡不掉落，不分裂
        MarkForDeletion();
        return;
    }

    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    for (int i = 0; i < SPLIT_COUNT; ++i) {
        IceSlime* small = em.SpawnEntity<IceSlime>();
        small->SetAsSmallSlime();
        small->SetPosition(m_position.x + RandomFloat(-20, 20), m_position.y + RandomFloat(-20, 20));
        small->SetTarget(m_pTarget);
    }
    DropItem* gold = em.SpawnEntity<DropItem>();
    gold->SetPosition(m_position);
    gold->SetDropType(DropType::GOLD);
    gold->SetValue(m_goldDrop);
    TrySpawnPotionDrop(m_position, em);
    MarkForDeletion();
}

// ============================================================
// Snowman - 雪人 (Ice Dungeon)
// ============================================================
Snowman::Snowman() {
    m_damage         = 2;
    m_attackRange    = 380.0f;
    m_attackCooldown = 1.25f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 4;
    m_moveSpeed      = 82.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(10);
    SetHP(10);
    SetAIState(EnemyState::IDLE);
}

void Snowman::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        Vector2 toTarget = GetDirectionToTarget();
        if (m_aiDecisionTimer <= 0.0f) {
            m_aiDecisionTimer = RandomFloat(0.8f, 1.3f);
            float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
            m_tacticalDir = Perpendicular(toTarget) * side;
        }
        if (dist < 190.0f) {
            m_moveDirection = ClampRoomVector(m_position, (m_position - m_pTarget->GetPosition()).Normalized() + m_tacticalDir * 0.2f);
        } else if (dist > 310.0f) {
            m_moveDirection = ClampRoomVector(m_position, toTarget * 0.45f + m_tacticalDir * 0.4f);
        } else {
            m_moveDirection = ClampRoomVector(m_position, m_tacticalDir * 0.55f);
        }
        FireBullet(230.0f, 0x00FFCC, 2);  // 亮青色雪球
    } else {
        SetAIState(EnemyState::IDLE);
    }
}

void Snowman::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(18.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 48 : 38;
    int h = m_isElite ? 50 : 42;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(235, 240, 239);
    DrawPixelHumanoid(cx, cy + 3, w, h, body, RGB(33, 43, 48), RGB(24, 28, 32));
    VisualFX::DrawPixelRect(cx - 14, cy - h / 2 - 16, cx + 14, cy - h / 2 - 5,
                            RGB(34, 38, 43), RGB(12, 14, 16), 3);
    setfillcolor(RGB(255, 131, 45));
    solidrectangle(cx - 3, cy - h / 2 + 9, cx + 10, cy - h / 2 + 13);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    VisualFX::DrawGlowCircle(cx, cy, r, RGB(190, 228, 244), 2);
    setfillcolor(RGB(30, 40, 50));
    solidcircle(cx, cy, r + 3);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(240, 240, 250));
    solidcircle(cx, cy, r);
    setfillcolor(RGB(255, 125, 48));
    solidcircle(cx, cy + 1, 3);
    setfillcolor(RGB(20, 20, 20));
    solidrectangle(cx - 10, cy - r - 6, cx + 10, cy - r + 2);
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(0, 0, 0));
        solidcircle(cx - 4, cy - 4, 2);
        solidcircle(cx + 4, cy - 4, 2);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 16;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

// ============================================================
// IceMage - 寒冰法师 (Ice Dungeon)
// ============================================================
IceMage::IceMage()
    : m_burstTimer(0.0f), m_burstCount(4), m_burstInterval(0.15f), m_burstAngleSpread(30.0f) {
    m_damage         = 2;
    m_attackRange    = 430.0f;
    m_attackCooldown = 1.65f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 72.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(7);
    SetHP(7);
    SetAIState(EnemyState::IDLE);
}

void IceMage::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        Vector2 toTarget = GetDirectionToTarget();
        if (dist < m_attackRange * 0.7f) {
            SetAIState(EnemyState::CHASE);
            m_moveDirection = ClampRoomVector(m_position, (m_position - m_pTarget->GetPosition()).Normalized());
        } else {
            SetAIState(EnemyState::CHASE);
            if (m_aiDecisionTimer <= 0.0f) {
                m_aiDecisionTimer = RandomFloat(0.9f, 1.5f);
                float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
                m_tacticalDir = Perpendicular(toTarget) * side;
            }
            m_moveDirection = ClampRoomVector(m_position, m_tacticalDir * 0.45f);
            // 扇形冰刺：每轮发射多枚子弹
            if (CanAttack()) {
                EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
                Vector2 aimPos = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 42.0f;
                Vector2 baseDir = (aimPos - m_position).Normalized();
                float baseAngle = std::atan2(baseDir.y, baseDir.x);
                int burstCount = 6;
                float spreadRad = (m_burstAngleSpread + 10.0f) * 3.14159f / 180.0f;
                for (int i = 0; i < burstCount; ++i) {
                    float offset = (i - (burstCount - 1) / 2.0f) * (spreadRad / (burstCount - 1));
                    float angle = baseAngle + offset;
                    Bullet* bullet = entityMgr.SpawnEntity<Bullet>();
                    bullet->SetPosition(m_position);
                    bullet->SetDirection(angle);
                    bullet->SetSpeed(210.0f);
                    bullet->SetDamage(2);  // 每发冰刺 2 点伤害
                    bullet->SetFaction(BulletFaction::ENEMY);
                    bullet->SetColor(0x00FFFF);  // 亮青色冰刺
                    bullet->SetCollisionSize(5.0f, 5.0f);
                    bullet->SetCausesSlow(true);
                    bullet->SetSlowDuration(2.0f);
                }
                m_attackTimer = m_attackCooldown;
            }
        }
    } else {
        SetAIState(EnemyState::IDLE);
    }
}

void IceMage::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(13.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 42 : 34;
    int h = m_isElite ? 46 : 38;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(62, 142, 205);
    DrawPixelHumanoid(cx, cy, w, h, body, RGB(14, 37, 64), RGB(205, 250, 255));
    VisualFX::DrawPixelDiamond(cx, cy - h / 2 - 13, 11, RGB(147, 229, 249), RGB(16, 54, 77));
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    VisualFX::DrawGlowCircle(cx, cy, r, RGB(92, 190, 236), 4);
    setfillcolor(RGB(10, 30, 60));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(60, 140, 200));
    solidcircle(cx, cy, r);
    setlinecolor(RGB(204, 248, 255));
    line(cx - 11, cy + 13, cx, cy - 18);
    line(cx + 11, cy + 13, cx, cy - 18);
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(200, 255, 255));
        solidcircle(cx, cy - 4, 4);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

// ============================================================
// FrostScout - 霜刃游卫 (Ice Dungeon)
// ============================================================
FrostScout::FrostScout()
    : m_skateTimer(0.0f)
    , m_sideSign(RandomInt(0, 1) == 0 ? -1.0f : 1.0f) {
    m_damage         = 2;
    m_attackRange    = 410.0f;
    m_attackCooldown = 1.05f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 158.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(6);
    SetHP(6);
    SetCollisionSize(12.0f, 12.0f);
    SetAIState(EnemyState::IDLE);
}

void FrostScout::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();
    if (dist >= m_detectionRange) {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
        return;
    }

    SetAIState(EnemyState::CHASE);
    m_skateTimer -= deltaTime;
    if (m_skateTimer <= 0.0f) {
        m_skateTimer = RandomFloat(0.45f, 0.75f);
        m_sideSign *= -1.0f;
    }

    Vector2 toTarget = GetDirectionToTarget();
    Vector2 side = Perpendicular(toTarget) * m_sideSign;
    if (dist < 150.0f) {
        m_moveDirection = ClampRoomVector(m_position,
            (m_position - m_pTarget->GetPosition()).Normalized() + side * 0.45f);
    } else if (dist > 320.0f) {
        m_moveDirection = ClampRoomVector(m_position, toTarget * 0.65f + side * 0.35f);
    } else {
        m_moveDirection = ClampRoomVector(m_position, side * 0.9f + toTarget * 0.18f);
    }

    if (dist < m_attackRange && CanAttack()) {
        EntityManager& em = GameManager::GetInstance().GetEntityManager();
        Vector2 lead = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 48.0f;
        Vector2 aim = (lead - m_position).Normalized();
        float baseAngle = std::atan2(aim.y, aim.x);
        for (int i = 0; i < 2; ++i) {
            float offset = (i == 0 ? -1.0f : 1.0f) * DegToRad(7.0f);
            Bullet* b = em.SpawnEntity<Bullet>();
            b->SetPosition(m_position.x + aim.x * 14.0f + side.x * (i == 0 ? -5.0f : 5.0f),
                           m_position.y + aim.y * 14.0f + side.y * (i == 0 ? -5.0f : 5.0f));
            b->SetDirection(baseAngle + offset);
            b->SetSpeed(255.0f);
            b->SetDamage(2);
            b->SetLifetime(3.5f);
            b->SetFaction(BulletFaction::ENEMY);
            b->SetColor(0xAEEFFF);
            b->SetCollisionSize(5.0f, 5.0f);
            b->SetCausesSlow(true);
            b->SetSlowDuration(1.4f);
        }
        m_attackTimer = m_attackCooldown;
    }
}

void FrostScout::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 42 : 34;
    int h = m_isElite ? 42 : 34;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(93, 177, 222);
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 7);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2 + 6, cx + w / 2, cy + h / 2,
                            body, RGB(14, 43, 67), 5);
    VisualFX::DrawPixelDiamond(cx, cy - h / 2 + 1, 15,
                               RGB(190, 244, 255), RGB(17, 58, 86));
    setfillcolor(RGB(225, 252, 255));
    solidrectangle(cx - 13, cy + h / 2 - 3, cx - 2, cy + h / 2 + 2);
    solidrectangle(cx + 2, cy + h / 2 - 3, cx + 13, cy + h / 2 + 2);
    setfillcolor(RGB(11, 21, 30));
    solidrectangle(cx - 7, cy - 5, cx - 4, cy - 2);
    solidrectangle(cx + 4, cy - 5, cx + 7, cy - 2);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
}

// ============================================================
// LavaWorm - 熔岩虫 (Volcano)
// ============================================================
LavaWorm::LavaWorm()
    : m_trailTimer(0.0f), m_trailDuration(3.0f) {
    m_damage         = 2;
    m_attackRange    = 30.0f;
    m_attackCooldown = 0.8f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 3;
    m_moveSpeed      = 100.0f;
    m_biome          = BiomeType::VOLCANO;
    SetMaxHP(5);
    SetHP(5);
    SetAIState(EnemyState::CHASE);
}

void LavaWorm::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        m_patrolAngle += deltaTime * 5.0f;
        Vector2 toTarget = GetDirectionToTarget();
        Vector2 weave = Perpendicular(toTarget) * std::sin(m_patrolAngle) * 0.38f;
        m_moveDirection = ClampRoomVector(m_position, toTarget + weave);
        if (dist < m_attackRange) Attack();
        m_trailTimer += deltaTime;
        if (m_trailTimer > 0.3f) {
            m_trailTimer = 0.0f;
            EntityManager& em = GameManager::GetInstance().GetEntityManager();
            LavaPool* pool = em.SpawnEntity<LavaPool>();
            pool->SetPosition(m_position);
        }
    } else {
        SetAIState(EnemyState::IDLE);
    }
}

void LavaWorm::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(10.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(221, 82, 34);
    VisualFX::DrawPixelShadow(cx, cy + 14, 30, 7);
    for (int i = 0; i < 3; ++i) {
        int ox = (i - 1) * 16;
        VisualFX::DrawPixelRect(cx + ox - 12, cy - 12 + std::abs(i - 1) * 4,
                                cx + ox + 12, cy + 12 + std::abs(i - 1) * 4,
                                i == 1 ? body : VisualFX::ScaleColor(body, 0.82f),
                                RGB(47, 17, 12), 4);
    }
    setfillcolor(RGB(255, 216, 72));
    solidrectangle(cx - 4, cy - 5, cx + 5, cy + 3);
    DrawEnemyHP(cx, cy, 25, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    VisualFX::DrawGlowCircle(cx, cy, r, RGB(255, 90, 32), 3);
    setfillcolor(RGB(40, 10, 5));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 100, 40) : RGB(220, 80, 20));
    solidcircle(cx, cy, r);
    if (!(flash && fp == 1)) {
        setfillcolor(RGB(255, 200, 60));
        solidcircle(cx, cy, 3);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

// ============================================================
// FireKnight - 火焰骑士 (Volcano)
// ============================================================
FireKnight::FireKnight()
    : m_shieldUp(true), m_shieldAngle(90.0f) {
    m_damage         = 2;
    m_attackRange    = 35.0f;
    m_attackCooldown = 0.9f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 6;
    m_moveSpeed      = 104.0f;
    m_biome          = BiomeType::VOLCANO;
    SetMaxHP(10);
    SetHP(10);
    SetAIState(EnemyState::CHASE);
}

void FireKnight::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();

    // 盾牌缓慢旋转追踪玩家（每秒 90 度），玩家可以绕开
    Vector2 targetDir = GetDirectionToTarget();
    float currentAngle = std::atan2(m_facingDirection.y, m_facingDirection.x);
    float targetAngle = std::atan2(targetDir.y, targetDir.x);
    float angleDiff = targetAngle - currentAngle;
    // 规范化到 [-PI, PI]
    while (angleDiff > PI) angleDiff -= 2.0f * PI;
    while (angleDiff < -PI) angleDiff += 2.0f * PI;
    float maxTurn = DegToRad(50.0f) * deltaTime;  // 50 度/秒
    if (angleDiff > maxTurn) angleDiff = maxTurn;
    else if (angleDiff < -maxTurn) angleDiff = -maxTurn;
    float newAngle = currentAngle + angleDiff;
    m_facingDirection = Vector2(std::cos(newAngle), std::sin(newAngle));

    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        Vector2 pressure = targetDir;
        if (dist < 105.0f) {
            float side = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
            pressure = (targetDir * 0.35f + Perpendicular(targetDir) * side * 0.65f).Normalized();
        }
        m_moveDirection = ClampRoomVector(m_position, pressure);
        if (dist < m_attackRange) Attack();
    } else {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
    }
}

void FireKnight::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    float scale = m_isElite ? m_eliteScale : 1.0f;
    int r = (int)(16.0f * scale);
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 48 : 38;
    int h = m_isElite ? 48 : 40;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(203, 82, 39);
    DrawPixelHumanoid(cx, cy, w, h, body, RGB(50, 22, 13), RGB(255, 225, 140));
    if (m_shieldUp) {
        int sx = cx + (int)(m_facingDirection.x * 24);
        int sy = cy + (int)(m_facingDirection.y * 24);
        VisualFX::DrawPixelRect(sx - 10, sy - 14, sx + 10, sy + 14,
                                RGB(222, 141, 45), RGB(52, 30, 13), 4);
    }
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    if (m_isElite) VisualFX::DrawGlowCircle(cx, cy, r, RGB(255, 118, 42), 4);
    setfillcolor(RGB(40, 15, 5));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(200, 70, 20));
    solidcircle(cx, cy, r);
    if (m_shieldUp) {
        int sx = cx + (int)(m_facingDirection.x * 18);
        int sy = cy + (int)(m_facingDirection.y * 18);
        setfillcolor(RGB(255, 140, 30));
        solidrectangle(sx - 6, sy - 10, sx + 6, sy + 10);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

void FireKnight::TakeDamage(int damage) {
    if (m_isDead) return;
    if (m_shieldUp) {
        Vector2 toAttacker = m_facingDirection;
        if (RandomFloat(0, 1) < 0.5f) return;
    }
    Enemy::TakeDamage(damage);
}

// ============================================================
// ExplosiveBat - 自爆蝙蝠 (Volcano)
// ============================================================
ExplosiveBat::ExplosiveBat()
    : m_explosionRadius(60.0f), m_isExploding(false), m_explosionTimer(0.0f)
    , m_ignoreObstacles(true) {
    m_damage         = 2;
    m_attackRange    = 30.0f;
    m_attackCooldown = 2.0f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 2;
    m_moveSpeed      = 140.0f;
    m_biome          = BiomeType::VOLCANO;
    SetMaxHP(3);
    SetHP(3);
    SetAIState(EnemyState::CHASE);
}

void ExplosiveBat::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (m_isExploding) {
        m_explosionTimer -= deltaTime;
        if (m_explosionTimer <= 0.0f) Die();
        return;
    }
    if (dist < m_detectionRange) {
        m_patrolAngle += deltaTime * 8.0f;
        Vector2 toTarget = GetDirectionToTarget();
        m_moveDirection = ClampRoomVector(m_position, toTarget + Perpendicular(toTarget) * std::sin(m_patrolAngle) * 0.55f);
        if (dist < m_attackRange) {
            m_isExploding = true;
            m_explosionTimer = 0.3f;
            m_moveDirection = Vector2(0.0f, 0.0f);
        }
    }
}

void ExplosiveBat::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    int r = m_isExploding ? 18 : 10;
    COLORREF body = m_isExploding ? RGB(255, 180, 20) : RGB(180, 40, 20);
    int w = m_isExploding ? 48 : 34;
    int h = m_isExploding ? 38 : 28;
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 7);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2,
                            body, RGB(45, 16, 12), 4);
    setfillcolor(VisualFX::ScaleColor(body, 0.72f));
    solidrectangle(cx - w / 2 - 16, cy - 4, cx - w / 2 + 4, cy + 8);
    solidrectangle(cx + w / 2 - 4, cy - 4, cx + w / 2 + 16, cy + 8);
    setfillcolor(RGB(255, 235, 135));
    solidrectangle(cx - 7, cy - 5, cx - 3, cy - 1);
    solidrectangle(cx + 3, cy - 5, cx + 7, cy - 1);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
    return;
    DrawEnemyShadow(cx, cy, r);
    if (m_isExploding) VisualFX::DrawGlowCircle(cx, cy, r, RGB(255, 160, 35), 5);
    setfillcolor(RGB(30, 10, 5));
    solidcircle(cx, cy, r + 2);
    setfillcolor(body);
    solidcircle(cx, cy, r);
    if (!m_isExploding) {
        setfillcolor(RGB(120, 30, 15));
        solidrectangle(cx - 14, cy - 4, cx - 6, cy + 4);
        solidrectangle(cx + 6, cy - 4, cx + 14, cy + 4);
    }
    if (m_hp < m_maxHP) {
        int barW = r * 2 + 4, barH = 4;
        int barX = cx - barW / 2, barY = cy - r - 10;
        float hpR = (float)m_hp / m_maxHP;
        setfillcolor(RGB(40, 40, 40));
        solidrectangle(barX, barY, barX + barW, barY + barH);
        setfillcolor(hpR > 0.5f ? RGB(60, 200, 60) : hpR > 0.25f ? RGB(220, 180, 40) : RGB(220, 40, 40));
        solidrectangle(barX, barY, barX + (int)(barW * hpR), barY + barH);
    }
}

void ExplosiveBat::Die() {
    if (m_isDead) return;
    m_isDead = true;
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    em.SpawnExplosion(m_position, m_explosionRadius, m_damage, BulletFaction::ENEMY);
    DropItem* gold = em.SpawnEntity<DropItem>();
    gold->SetPosition(m_position);
    gold->SetDropType(DropType::GOLD);
    gold->SetValue(m_goldDrop);
    TrySpawnPotionDrop(m_position, em);
    MarkForDeletion();
}

// ============================================================
// EmberImp - 余烬小鬼 (Volcano)
// ============================================================
EmberImp::EmberImp()
    : m_phaseTimer(RandomFloat(0.0f, PI))
    , m_zigzag(RandomInt(0, 1) == 0 ? -1.0f : 1.0f) {
    m_damage         = 2;
    m_attackRange    = 430.0f;
    m_attackCooldown = 1.12f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 150.0f;
    m_biome          = BiomeType::VOLCANO;
    SetMaxHP(6);
    SetHP(6);
    SetCollisionSize(12.0f, 12.0f);
    SetAIState(EnemyState::IDLE);
}

void EmberImp::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;

    float dist = GetDistanceToTarget();
    if (dist >= m_detectionRange) {
        SetAIState(EnemyState::IDLE);
        m_moveDirection = Vector2(0.0f, 0.0f);
        return;
    }

    SetAIState(EnemyState::CHASE);
    m_phaseTimer += deltaTime * 5.2f;
    if (m_aiDecisionTimer <= 0.0f) {
        m_aiDecisionTimer = RandomFloat(0.55f, 0.95f);
        m_zigzag = RandomInt(0, 1) == 0 ? -1.0f : 1.0f;
    }

    Vector2 toTarget = GetDirectionToTarget();
    Vector2 side = Perpendicular(toTarget) * (std::sin(m_phaseTimer) >= 0.0f ? 1.0f : -1.0f) * m_zigzag;
    if (dist < 190.0f) {
        m_moveDirection = ClampRoomVector(m_position,
            (m_position - m_pTarget->GetPosition()).Normalized() + side * 0.62f);
    } else if (dist > 330.0f) {
        m_moveDirection = ClampRoomVector(m_position, toTarget * 0.62f + side * 0.42f);
    } else {
        m_moveDirection = ClampRoomVector(m_position, side * 0.72f + toTarget * 0.18f);
    }

    if (dist < m_attackRange && CanAttack()) {
        EntityManager& em = GameManager::GetInstance().GetEntityManager();
        Vector2 lead = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 42.0f;
        Vector2 aim = (lead - m_position).Normalized();
        float baseAngle = std::atan2(aim.y, aim.x);
        for (int i = 0; i < 2; ++i) {
            float offset = (i == 0 ? -1.0f : 1.0f) * DegToRad(10.0f);
            Bullet* b = em.SpawnEntity<Bullet>();
            b->SetPosition(m_position.x + aim.x * 13.0f + side.x * (i == 0 ? -7.0f : 7.0f),
                           m_position.y + aim.y * 13.0f + side.y * (i == 0 ? -7.0f : 7.0f));
            b->SetDirection(baseAngle + offset);
            b->SetSpeed(285.0f);
            b->SetDamage(2);
            b->SetLifetime(3.8f);
            b->SetFaction(BulletFaction::ENEMY);
            b->SetColor(0xFF7A2C);
            b->SetCollisionSize(6.0f, 6.0f);
            b->SetCausesBurning(true);
            b->SetBurnChance(0.35f);
            b->SetBurnDuration(1.8f);
        }
        m_attackTimer = m_attackCooldown;
    }
}

void EmberImp::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    bool flash = (m_damageFlashTimer > 0.0f);
    int fp = (int)(m_damageFlashTimer * 30) % 2;
    int w = m_isElite ? 42 : 34;
    int h = m_isElite ? 43 : 35;
    COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(211, 68, 45);
    VisualFX::DrawPixelShadow(cx, cy + h / 2, w / 2, 7);
    VisualFX::DrawPixelRect(cx - w / 2, cy - h / 2 + 4, cx + w / 2, cy + h / 2,
                            body, RGB(49, 17, 13), 5);
    VisualFX::DrawPixelRect(cx - w / 2 + 6, cy - h / 2 - 9,
                            cx + w / 2 - 6, cy - h / 2 + 12,
                            VisualFX::ScaleColor(body, 1.12f), RGB(49, 17, 13), 5);
    setfillcolor(RGB(255, 208, 76));
    solidrectangle(cx - 13, cy - h / 2 - 15, cx - 6, cy - h / 2 - 4);
    solidrectangle(cx + 6, cy - h / 2 - 15, cx + 13, cy - h / 2 - 4);
    setfillcolor(RGB(255, 230, 128));
    solidrectangle(cx - 8, cy - 3, cx - 4, cy + 1);
    solidrectangle(cx + 4, cy - 3, cx + 8, cy + 1);
    VisualFX::DrawGlowCircle(cx, cy + 12, 10, RGB(255, 112, 36), 2);
    DrawEnemyHP(cx, cy, w / 2, m_hp, m_maxHP);
}

// ============================================================
// Mimic - 伪装宝箱怪
// ============================================================
Mimic::Mimic()
    : m_isDisguised(true), m_hasTriggered(false), m_revealTimer(0.0f) {
    m_damage         = 2;
    m_attackRange    = 32.0f;
    m_attackCooldown = 0.7f;
    m_detectionRange = 1600.0f;
    m_goldDrop       = 8;
    m_moveSpeed      = 130.0f;
    m_biome          = BiomeType::FOREST;
    SetMaxHP(10);
    SetHP(10);
    SetAIState(EnemyState::IDLE);
}

void Mimic::UpdateAI(float deltaTime) {
    if (m_isDisguised && !m_hasTriggered) return;
    if (!m_pTarget || m_pTarget->IsDead()) return;
    m_revealTimer -= deltaTime;
    if (m_revealTimer <= 0.0f && m_isDisguised) Reveal();
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        m_moveDirection = GetDirectionToTarget();
        if (dist < m_attackRange) Attack();
    }
}

void Mimic::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x, cy = (int)m_position.y;
    int r = 14;
    if (m_isDisguised) {
        VisualFX::DrawPixelShadow(cx, cy + 18, 24, 8);
        VisualFX::DrawPixelRect(cx - 24, cy - 15, cx + 24, cy + 17,
                                RGB(182, 112, 43), RGB(29, 20, 16), 5);
        setfillcolor(RGB(230, 171, 58));
        solidrectangle(cx - 16, cy - 6, cx + 16, cy + 8);
        VisualFX::DrawPixelRect(cx - 6, cy - 2, cx + 6, cy + 8,
                                RGB(238, 195, 63), RGB(60, 39, 12), 3);
    } else {
        bool flash = (m_damageFlashTimer > 0.0f);
        int fp = (int)(m_damageFlashTimer * 30) % 2;
        COLORREF body = (flash && fp == 1) ? RGB(238, 75, 80) : RGB(151, 58, 183);
        DrawPixelHumanoid(cx, cy, 40, 38, body, RGB(38, 18, 48), RGB(255, 245, 230));
        setfillcolor(RGB(255, 240, 210));
        solidrectangle(cx - 12, cy + 9, cx + 12, cy + 14);
        setfillcolor(RGB(40, 18, 48));
        solidrectangle(cx - 8, cy + 9, cx - 5, cy + 14);
        solidrectangle(cx + 5, cy + 9, cx + 8, cy + 14);
        DrawEnemyHP(cx, cy, 20, m_hp, m_maxHP);
    }
}

void Mimic::Reveal() {
    m_isDisguised = false;
    m_hasTriggered = true;
    printf("[Mimic] Revealed!\n");
}
