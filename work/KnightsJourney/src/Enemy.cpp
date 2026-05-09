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
#include <graphics.h>
#include <cmath>
#include <cstdio>

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
    , m_idleWanderDir(0.0f, 0.0f) {

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
    const float margin = 18.0f;
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
    m_detectionRange = 400.0f;
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
        m_moveDirection = GetDirectionToTarget();
        if (dist < m_attackRange) {
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
    int radius = static_cast<int>(14.0f * scale);

    bool flashRed = (m_damageFlashTimer > 0.0f);
    int flashPhase = static_cast<int>(m_damageFlashTimer * 30) % 2;

    // 外圈描边（深绿）
    setfillcolor(flashRed ? RGB(60, 10, 10) : RGB(20, 60, 20));
    solidcircle(cx, cy, radius + 2);

    // 主体（哥布林绿）
    COLORREF bodyColor;
    if (flashRed && flashPhase == 1) {
        bodyColor = RGB(255, 60, 60);
    } else if (m_isElite) {
        bodyColor = RGB(200, 120, 20);   // 精英：橙色
    } else {
        bodyColor = RGB(80, 180, 60);    // 普通：绿色
    }
    setfillcolor(bodyColor);
    solidcircle(cx, cy, radius);

    // 眼睛（白色小圆）
    if (!(flashRed && flashPhase == 1)) {
        setfillcolor(RGB(255, 255, 255));
        solidcircle(cx + 4, cy - 5, 3);
        solidcircle(cx - 4, cy - 5, 3);
        // 瞳孔
        setfillcolor(RGB(20, 20, 20));
        solidcircle(cx + 5, cy - 5, 1);
        solidcircle(cx - 3, cy - 5, 1);
    }

    // HP 条
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
// GoblinArcher - 哥布林弓箭手 (Forest)
// ============================================================
GoblinArcher::GoblinArcher()
    : m_preferredDistance(250.0f) {
    m_damage         = 2;
    m_attackRange    = 350.0f;
    m_attackCooldown = 1.5f;
    m_detectionRange = 450.0f;
    m_goldDrop       = 4;
    m_moveSpeed      = 70.0f;
    m_biome          = BiomeType::FOREST;
    SetMaxHP(4);
    SetHP(4);
    SetAIState(EnemyState::IDLE);
}

void GoblinArcher::UpdateAI(float deltaTime) {
    (void)deltaTime;
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        if (dist < m_preferredDistance * 0.6f) {
            m_moveDirection = (m_position - m_pTarget->GetPosition()).Normalized();
        } else if (dist > m_preferredDistance * 1.2f) {
            m_moveDirection = GetDirectionToTarget();
        } else {
            m_moveDirection = Vector2(0.0f, 0.0f);
            FireBullet(160.0f, 0xCC8844, 2);  // 棕色箭矢，2点伤害
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
    setfillcolor(flash ? RGB(60, 10, 10) : RGB(40, 30, 10));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(160, 130, 60));
    solidcircle(cx, cy, r);
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
    , m_stunTimer(0.0f), m_rushSpeed(280.0f) {
    m_damage         = 2;
    m_attackRange    = 40.0f;
    m_attackCooldown = 1.0f;
    m_detectionRange = 400.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 80.0f;
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
                m_boarState = BoarState::CHARGING;
                m_chargeTimer = 0.8f;
                m_rushDirection = GetDirectionToTarget();
            }
            break;
        case BoarState::CHARGING:
            m_chargeTimer -= deltaTime;
            if (m_chargeTimer <= 0.0f) { m_boarState = BoarState::RUSHING; m_rushTimer = 0.8f; }
            break;
        case BoarState::RUSHING:
            m_rushTimer -= deltaTime;
            m_position += m_rushDirection * m_rushSpeed * deltaTime;
            if (m_rushTimer <= 0.0f) { m_boarState = BoarState::STUNNED_AFTER_WALL; m_stunTimer = 0.5f; }
            break;
        case BoarState::STUNNED_AFTER_WALL:
            m_stunTimer -= deltaTime;
            m_moveDirection = Vector2(0.0f, 0.0f);
            if (m_stunTimer <= 0.0f) m_boarState = BoarState::IDLE;
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
    setfillcolor(flash ? RGB(50, 10, 5) : RGB(40, 20, 5));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(140, 60, 20));
    solidcircle(cx, cy, r);
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
// IceSlime - 冰原史莱姆 (Ice Dungeon)
// ============================================================
IceSlime::IceSlime() : m_isSmall(false) {
    m_damage         = 2;
    m_attackRange    = 30.0f;
    m_attackCooldown = 1.2f;
    m_detectionRange = 300.0f;
    m_goldDrop       = 2;
    m_moveSpeed      = 60.0f;
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
        m_moveDirection = GetDirectionToTarget();
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
    setfillcolor(RGB(20, 40, 60));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 80, 80) : RGB(80, 180, 220));
    solidcircle(cx, cy, r);
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
    MarkForDeletion();
}

// ============================================================
// Snowman - 雪人 (Ice Dungeon)
// ============================================================
Snowman::Snowman() {
    m_damage         = 2;
    m_attackRange    = 300.0f;
    m_attackCooldown = 2.0f;
    m_detectionRange = 400.0f;
    m_goldDrop       = 4;
    m_moveSpeed      = 45.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(10);
    SetHP(10);
    SetAIState(EnemyState::IDLE);
}

void Snowman::UpdateAI(float deltaTime) {
    (void)deltaTime;
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        SetAIState(EnemyState::CHASE);
        m_moveDirection = Vector2(0.0f, 0.0f);
        FireBullet(120.0f, 0x00FFCC, 2);  // 亮青色雪球
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
    setfillcolor(RGB(30, 40, 50));
    solidcircle(cx, cy, r + 3);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(240, 240, 250));
    solidcircle(cx, cy, r);
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
    m_attackRange    = 350.0f;
    m_attackCooldown = 3.0f;
    m_detectionRange = 400.0f;
    m_goldDrop       = 5;
    m_moveSpeed      = 30.0f;
    m_biome          = BiomeType::ICE_DUNGEON;
    SetMaxHP(7);
    SetHP(7);
    SetAIState(EnemyState::IDLE);
}

void IceMage::UpdateAI(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    float dist = GetDistanceToTarget();
    if (dist < m_detectionRange) {
        if (dist < m_attackRange * 0.7f) {
            SetAIState(EnemyState::CHASE);
            m_moveDirection = (m_position - m_pTarget->GetPosition()).Normalized();
        } else {
            SetAIState(EnemyState::CHASE);
            m_moveDirection = Vector2(0.0f, 0.0f);
            // 扇形冰刺：每轮发射多枚子弹
            if (CanAttack()) {
                EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
                Vector2 baseDir = GetDirectionToTarget();
                float baseAngle = std::atan2(baseDir.y, baseDir.x);
                int burstCount = 5;
                float spreadRad = m_burstAngleSpread * 3.14159f / 180.0f;
                for (int i = 0; i < burstCount; ++i) {
                    float offset = (i - (burstCount - 1) / 2.0f) * (spreadRad / (burstCount - 1));
                    float angle = baseAngle + offset;
                    Bullet* bullet = entityMgr.SpawnEntity<Bullet>();
                    bullet->SetPosition(m_position);
                    bullet->SetDirection(angle);
                    bullet->SetSpeed(120.0f);
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
    setfillcolor(RGB(10, 30, 60));
    solidcircle(cx, cy, r + 2);
    setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(60, 140, 200));
    solidcircle(cx, cy, r);
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
// LavaWorm - 熔岩虫 (Volcano)
// ============================================================
LavaWorm::LavaWorm()
    : m_trailTimer(0.0f), m_trailDuration(3.0f) {
    m_damage         = 2;
    m_attackRange    = 30.0f;
    m_attackCooldown = 0.8f;
    m_detectionRange = 350.0f;
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
        m_moveDirection = GetDirectionToTarget();
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
    m_detectionRange = 350.0f;
    m_goldDrop       = 6;
    m_moveSpeed      = 80.0f;
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
        m_moveDirection = targetDir;  // 移动仍然直接追踪
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
    m_detectionRange = 400.0f;
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
        m_moveDirection = GetDirectionToTarget();
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
    MarkForDeletion();
}

// ============================================================
// Mimic - 伪装宝箱怪
// ============================================================
Mimic::Mimic()
    : m_isDisguised(true), m_hasTriggered(false), m_revealTimer(0.0f) {
    m_damage         = 2;
    m_attackRange    = 32.0f;
    m_attackCooldown = 0.7f;
    m_detectionRange = 200.0f;
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
        setfillcolor(RGB(100, 60, 15));
        solidrectangle(cx - 14, cy - 10, cx + 14, cy + 10);
        setfillcolor(RGB(180, 140, 30));
        solidrectangle(cx - 12, cy - 8, cx + 12, cy + 8);
        setfillcolor(RGB(255, 215, 0));
        solidcircle(cx, cy, 4);
    } else {
        bool flash = (m_damageFlashTimer > 0.0f);
        int fp = (int)(m_damageFlashTimer * 30) % 2;
        setfillcolor(RGB(30, 10, 30));
        solidcircle(cx, cy, r + 2);
        setfillcolor(flash && fp == 1 ? RGB(255, 60, 60) : RGB(150, 40, 180));
        solidcircle(cx, cy, r);
        setfillcolor(RGB(255, 255, 255));
        solidcircle(cx - 4, cy - 4, 4);
        solidcircle(cx + 4, cy - 4, 4);
    }
}

void Mimic::Reveal() {
    m_isDisguised = false;
    m_hasTriggered = true;
    printf("[Mimic] Revealed!\n");
}
