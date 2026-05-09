// ============================================================
// Boss.cpp - Boss 基类 + 3种Boss实现
// ============================================================

#include "../include/Entity/Boss.h"
#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/Obstacle.h"
#include "../include/Entity/DropItem.h"
#include "../include/System/EntityManager.h"
#include "../include/Core/GameManager.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>

// ============================================================
// Boss 基类
// ============================================================
Boss::Boss()
    : m_phase(BossPhase::PHASE_1), m_pTarget(nullptr)
    , m_skill1Cooldown(6.0f), m_skill1Timer(0.0f)
    , m_skill2Cooldown(12.0f), m_skill2Timer(0.0f)
    , m_skillCastTimer(0.0f), m_isCasting(false)
    , m_halfHpMpDropped(false) {
    SetMaxHP(80);
    SetHP(80);
    SetMoveSpeed(80.0f);
    SetRenderLayer(7);
    SetCollisionSize(24.0f, 24.0f);
}

void Boss::Update(float deltaTime) {
    if (m_isDead) return;
    UpdateStatusEffects(deltaTime);
    UpdateSkillTimers(deltaTime);
    UpdatePhaseBehavior(deltaTime);
    SyncAABBToPosition();
}

void Boss::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    int r = m_phase == BossPhase::ENRAGED ? 28 : 22;

    // Boss 血条
    int barW = 80;
    int barH = 6;
    int barX = cx - barW / 2;
    int barY = cy - r - 16;
    float hpRatio = (float)m_hp / m_maxHP;

    setfillcolor(RGB(40, 40, 40));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    setfillcolor(hpRatio > 0.5f ? RGB(200, 40, 40) : RGB(255, 180, 20));
    solidrectangle(barX, barY, barX + (int)(barW * hpRatio), barY + barH);

    // 红色描边
    setfillcolor(RGB(50, 10, 10));
    solidcircle(cx, cy, r + 3);

    // 主体：紫色调
    COLORREF body = (m_phase == BossPhase::ENRAGED) ? RGB(220, 60, 40) : RGB(160, 40, 120);
    setfillcolor(body);
    solidcircle(cx, cy, r);
}

void Boss::TakeDamage(int damage) {
    if (m_isDead) return;
    m_hp -= damage;
    if (m_hp <= 0) { m_hp = 0; Die(); }

    // 半血时掉落蓝量补给（只触发一次）
    if (!m_halfHpMpDropped && m_hp <= m_maxHP / 2) {
        m_halfHpMpDropped = true;
        EntityManager& em = GameManager::GetInstance().GetEntityManager();
        for (int i = 0; i < 3; ++i) {
            DropItem* mp = em.SpawnEntity<DropItem>();
            mp->SetPosition(m_position.x + RandomFloat(-50, 50),
                           m_position.y + RandomFloat(-50, 50));
            mp->SetDropType(DropType::MP_POTION);
            mp->SetValue(50);
        }
        printf("[Boss] Half HP reached - MP potions dropped!\n");
    }

    float ratio = (float)m_hp / m_maxHP;
    if (ratio < 0.33f) SetPhase(BossPhase::ENRAGED);
    else if (ratio < 0.50f) SetPhase(BossPhase::PHASE_2);
}

void Boss::Die() {
    m_isDead = true;
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    // 掉落大量金币
    for (int i = 0; i < 5; ++i) {
        DropItem* gold = em.SpawnEntity<DropItem>();
        gold->SetPosition(m_position.x + RandomFloat(-30, 30), m_position.y + RandomFloat(-30, 30));
        gold->SetDropType(DropType::GOLD);
        gold->SetValue(10);
    }
    // 掉落大量蓝瓶（MP 回复）
    for (int i = 0; i < 4; ++i) {
        DropItem* mp = em.SpawnEntity<DropItem>();
        mp->SetPosition(m_position.x + RandomFloat(-40, 40), m_position.y + RandomFloat(-40, 40));
        mp->SetDropType(DropType::MP_POTION);
        mp->SetValue(50);
    }
    // 掉落 1-2 个血瓶
    for (int i = 0; i < 2; ++i) {
        DropItem* hp = em.SpawnEntity<DropItem>();
        hp->SetPosition(m_position.x + RandomFloat(-35, 35), m_position.y + RandomFloat(-35, 35));
        hp->SetDropType(DropType::HP_POTION);
        hp->SetValue(3);
    }
    MarkForDeletion();
}

void Boss::SetPhase(BossPhase phase) {
    if (m_phase == phase) return;
    m_phase = phase;
    printf("[Boss] Phase changed to %d\n", (int)phase);
}

float Boss::GetPhaseThreshold() const {
    switch (m_phase) {
        case BossPhase::PHASE_1: return 1.0f;
        case BossPhase::PHASE_2: return 0.50f;
        case BossPhase::ENRAGED: return 0.33f;
    }
    return 1.0f;
}

void Boss::UpdateSkillTimers(float deltaTime) {
    if (m_skill1Timer > 0.0f) m_skill1Timer -= deltaTime;
    if (m_skill2Timer > 0.0f) m_skill2Timer -= deltaTime;
    if (m_skillCastTimer > 0.0f) m_skillCastTimer -= deltaTime;
    else m_isCasting = false;
}

void Boss::UpdatePhaseBehavior(float deltaTime) {
    if (!m_pTarget || m_pTarget->IsDead()) return;
    // 朝向玩家移动
    Vector2 dir = m_pTarget->GetPosition() - m_position;
    m_moveDirection = dir.Normalized();
    m_position += m_moveDirection * m_moveSpeed * deltaTime;

    // 自动释放技能
    if (CanCastSkill1()) { CastSkill1(); m_skill1Timer = m_skill1Cooldown; }
    if (CanCastSkill2()) { CastSkill2(); m_skill2Timer = m_skill2Cooldown; }

    // 边界限制
    if (m_position.x < 30) m_position.x = 30;
    if (m_position.y < 30) m_position.y = 30;
    if (m_position.x > ROOM_WIDTH - 30) m_position.x = ROOM_WIDTH - 30;
    if (m_position.y > ROOM_HEIGHT - 30) m_position.y = ROOM_HEIGHT - 30;
}

// ============================================================
// Treant - 巨型树人 (Forest Boss)
// ============================================================
Treant::Treant()
    : m_shockwaveProjectileCount(16), m_shockwaveRadius(20.0f)
    , m_shockwaveSpeed(140.0f), m_summonCount(3) {
    m_skill1Cooldown = 5.0f;
    m_skill2Cooldown = 10.0f;
    SetMaxHP(100);
    SetHP(100);
}

void Treant::Update(float deltaTime) { Boss::Update(deltaTime); }

void Treant::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    int r = m_phase == BossPhase::ENRAGED ? 32 : 26;

    // 树干
    setfillcolor(RGB(60, 40, 20));
    solidrectangle(cx - 10, cy - 5, cx + 10, cy + 18);
    // 树冠
    COLORREF crown = (m_phase == BossPhase::ENRAGED) ? RGB(220, 60, 20) : RGB(30, 150, 50);
    setfillcolor(crown);
    solidcircle(cx, cy - 8, r);
    // 眼睛
    setfillcolor(RGB(255, 255, 0));
    solidcircle(cx - 6, cy - 4, 5);
    solidcircle(cx + 6, cy - 4, 5);
    setfillcolor(RGB(0, 0, 0));
    solidcircle(cx - 6, cy - 4, 2);
    solidcircle(cx + 6, cy - 4, 2);

    // HP 条
    int barW = 80, barH = 6;
    int barX = cx - barW / 2, barY = cy - r - 16;
    float hpRatio = (float)m_hp / m_maxHP;
    setfillcolor(RGB(40, 40, 40));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    setfillcolor(hpRatio > 0.5f ? RGB(200, 40, 40) : RGB(255, 180, 20));
    solidrectangle(barX, barY, barX + (int)(barW * hpRatio), barY + barH);
}

void Treant::CastSkill1() {
    // 震荡波：向周围发射弹幕
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    for (int i = 0; i < m_shockwaveProjectileCount; ++i) {
        float angle = (float)i * 2.0f * PI / m_shockwaveProjectileCount;
        Bullet* b = em.SpawnEntity<Bullet>();
        b->SetPosition(m_position);
        b->SetDirection(angle);
        b->SetSpeed(m_shockwaveSpeed);
        b->SetDamage(3);
        b->SetLifetime(2.0f);
        b->SetFaction(BulletFaction::ENEMY);
        b->SetColor(0x886644);
    }
}

void Treant::CastSkill2() {
    // 召唤哥布林
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    for (int i = 0; i < m_summonCount; ++i) {
        GoblinMelee* gob = em.SpawnEntity<GoblinMelee>();
        gob->SetPosition(m_position.x + RandomFloat(-60, 60), m_position.y + RandomFloat(-60, 60));
        gob->SetTarget(GameManager::GetInstance().GetPlayer());
    }
}

// ============================================================
// CrystalCrab - 水晶巨蟹 (Ice Boss)
// ============================================================
CrystalCrab::CrystalCrab()
    : m_isShelled(false), m_shellTimer(0.0f), m_shellHealPerSecond(8.0f)
    , m_laserAngle(0.0f), m_laserRotateSpeed(90.0f), m_laserCount(2) {
    m_skill1Cooldown = 3.0f;
    m_skill2Cooldown = 7.0f;
    SetMaxHP(220);
    SetHP(220);
}

void CrystalCrab::Update(float deltaTime) {
    Boss::Update(deltaTime);
    if (m_isShelled) {
        m_shellTimer -= deltaTime;
        m_hp = (int)(m_hp + m_shellHealPerSecond * deltaTime);
        if (m_hp > m_maxHP) m_hp = m_maxHP;
        if (m_shellTimer <= 0.0f) m_isShelled = false;
    }
}

void CrystalCrab::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    int r = m_phase == BossPhase::ENRAGED ? 28 : 22;

    COLORREF body = m_isShelled ? RGB(80, 200, 240) : RGB(40, 120, 200);
    setfillcolor(RGB(10, 30, 60));
    solidcircle(cx, cy, r + 3);
    setfillcolor(body);
    solidcircle(cx, cy, r);
    // 眼睛
    setfillcolor(RGB(255, 60, 60));
    solidcircle(cx - 6, cy - 4, 4);
    solidcircle(cx + 6, cy - 4, 4);

    int barW = 80, barH = 6;
    int barX = cx - barW / 2, barY = cy - r - 16;
    float hpRatio = (float)m_hp / m_maxHP;
    setfillcolor(RGB(40, 40, 40));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    setfillcolor(hpRatio > 0.5f ? RGB(40, 120, 220) : RGB(255, 180, 20));
    solidrectangle(barX, barY, barX + (int)(barW * hpRatio), barY + barH);
}

void CrystalCrab::TakeDamage(int damage) {
    if (m_isDead) return;
    if (m_isShelled) {
        // 缩壳时攻击反而回血
        m_hp = (int)(m_hp + damage * 0.5f);
        if (m_hp > m_maxHP) m_hp = m_maxHP;
        return;
    }
    Boss::TakeDamage(damage);
}

void CrystalCrab::CastSkill1() {
    // 交叉激光 + 朝向玩家散射冰刺
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    // 旋转激光
    for (int i = 0; i < m_laserCount; ++i) {
        float angle = m_laserAngle + (float)i * PI;
        for (int j = 0; j < 6; ++j) {
            Bullet* b = em.SpawnEntity<Bullet>();
            b->SetPosition(m_position.x + std::cos(angle) * 30, m_position.y + std::sin(angle) * 30);
            b->SetDirection(angle);
            b->SetSpeed(220.0f + j * 20.0f);
            b->SetDamage(2);
            b->SetLifetime(1.5f);
            b->SetFaction(BulletFaction::ENEMY);
            b->SetColor(0x44CCFF);
        }
    }
    m_laserAngle += DegToRad(m_laserRotateSpeed / 4.0f);

    // 额外朝向玩家发射扇形冰刺
    if (m_pTarget) {
        float aimAngle = std::atan2(m_pTarget->GetPosition().y - m_position.y,
                                    m_pTarget->GetPosition().x - m_position.x);
        int burstCount = 7;
        float spreadDeg = 40.0f;
        for (int i = 0; i < burstCount; ++i) {
            float t = (burstCount > 1) ? (float)i / (burstCount - 1) : 0.5f;
            float offset = (t - 0.5f) * DegToRad(spreadDeg);
            float a = aimAngle + offset;
            Bullet* b = em.SpawnEntity<Bullet>();
            b->SetPosition(m_position);
            b->SetDirection(a);
            b->SetSpeed(170.0f);
            b->SetDamage(1);
            b->SetLifetime(2.0f);
            b->SetFaction(BulletFaction::ENEMY);
            b->SetColor(0x88DDFF);
            b->SetCausesSlow(true);
            b->SetSlowDuration(1.5f);
        }
    }
}

void CrystalCrab::CastSkill2() {
    // 缩壳回血
    m_isShelled = true;
    m_shellTimer = 4.0f;
    printf("[CrystalCrab] Shelled! Healing...\n");
}

// ============================================================
// LavaDragon - 熔岩飞龙 (Volcano Boss)
// ============================================================
LavaDragon::LavaDragon()
    : m_isBurrowed(false)
    , m_burrowTimer(0.0f)
    , m_burrowTrailTimer(0.0f)
    , m_burrowEmergeRadius(90.0f)
    , m_burrowEmergeDamage(5)
    , m_fireBreathProjectileCount(20)
    , m_fireBreathSpreadAngle(60.0f) {
    m_skill1Cooldown = 3.0f;
    m_skill2Cooldown = 8.0f;
    m_skill1Timer = 2.0f;   // 2秒后才开始喷火
    m_skill2Timer = 5.0f;   // 5秒后才开始潜地
    SetMaxHP(495);
    SetHP(495);
    SetMoveSpeed(100.0f);
}

void LavaDragon::Update(float deltaTime) {
    if (m_isDead) return;
    UpdateStatusEffects(deltaTime);
    UpdateSkillTimers(deltaTime);

    if (!m_isBurrowed) {
        UpdatePhaseBehavior(deltaTime);
    } else {
        // 潜地追踪状态
        m_burrowTimer -= deltaTime;

        // 实时追踪玩家位置
        if (m_pTarget && !m_pTarget->IsDead()) {
            m_burrowTargetPos = m_pTarget->GetPosition();
        }

        // 向目标位置快速移动
        Vector2 toTarget = m_burrowTargetPos - m_position;
        float dist = toTarget.Length();
        if (dist > 5.0f) {
            m_moveDirection = toTarget.Normalized();
            m_position += m_moveDirection * m_moveSpeed * 1.8f * deltaTime;
        }

        // 每 0.15 秒留下一个轨迹点
        m_burrowTrailTimer -= deltaTime;
        if (m_burrowTrailTimer <= 0.0f) {
            m_burrowTrailTimer = 0.15f;
            m_burrowTrail.push_back(m_position);
            // 在当前位置生成岩浆池
            EntityManager& em = GameManager::GetInstance().GetEntityManager();
            LavaPool* pool = em.SpawnEntity<LavaPool>();
            pool->SetPosition(m_position.x + RandomFloat(-15, 15),
                             m_position.y + RandomFloat(-15, 15));
            pool->SetLifetime(4.0f);
        }

        // 限制轨迹点数量
        if (m_burrowTrail.size() > 20) {
            m_burrowTrail.erase(m_burrowTrail.begin());
        }

        // 到达目标附近或时间到 → 钻出
        bool reachedTarget = (dist < 40.0f);
        bool timeUp = (m_burrowTimer <= 0.0f);

        if (reachedTarget || timeUp) {
            m_isBurrowed = false;
            // 钻出范围伤害
            EntityManager& em = GameManager::GetInstance().GetEntityManager();
            em.SpawnExplosion(m_position, m_burrowEmergeRadius,
                             m_burrowEmergeDamage, BulletFaction::ENEMY);
            // 钻出位置生成一圈岩浆
            for (int i = 0; i < 8; ++i) {
                float angle = i * 2.0f * PI / 8;
                LavaPool* pool = em.SpawnEntity<LavaPool>();
                pool->SetPosition(m_position.x + std::cos(angle) * 40,
                                 m_position.y + std::sin(angle) * 40);
                pool->SetLifetime(3.0f);
            }
            m_burrowTrail.clear();
            printf("[LavaDragon] Emerged! Damage dealt in area.\n");
        }
    }

    // 边界限制
    if (m_position.x < 40) m_position.x = 40;
    if (m_position.y < 40) m_position.y = 40;
    if (m_position.x > ROOM_WIDTH - 40) m_position.x = ROOM_WIDTH - 40;
    if (m_position.y > ROOM_HEIGHT - 40) m_position.y = ROOM_HEIGHT - 40;

    SyncAABBToPosition();
}

void LavaDragon::Render() {
    if (m_isDead) return;

    int cx = (int)m_position.x;
    int cy = (int)m_position.y;

    if (m_isBurrowed) {
        // ---- 地面潜行渲染：可见的发光轨迹 ----
        // 绘制轨迹（从旧到新逐渐变亮）
        for (size_t i = 0; i < m_burrowTrail.size(); ++i) {
            float alpha = (float)i / m_burrowTrail.size();
            int r = (int)(8 + alpha * 12);
            int red = (int)(100 + alpha * 155);
            int green = (int)(20 + alpha * 80);
            int blue = (int)(alpha * 30);
            setfillcolor(RGB(red, green, blue));
            solidcircle((int)m_burrowTrail[i].x, (int)m_burrowTrail[i].y, r);
        }

        // 当前位置：明亮的地面隆起
        int pulseR = 16 + (int)(std::sin(m_burrowTimer * 8.0f) * 6.0f);  // 脉冲大小
        // 外圈光晕
        setfillcolor(RGB(255, 120, 20));
        solidcircle(cx, cy, pulseR + 6);
        setfillcolor(RGB(255, 180, 40));
        solidcircle(cx, cy, pulseR + 3);
        // 内核
        setfillcolor(RGB(255, 255, 100));
        solidcircle(cx, cy, pulseR > 10 ? pulseR - 4 : 6);

        // 裂缝效果（十字裂纹）
        setlinestyle(PS_SOLID, 2);
        setlinecolor(RGB(255, 100, 20));
        line(cx - pulseR - 4, cy, cx + pulseR + 4, cy);
        line(cx, cy - pulseR - 4, cx, cy + pulseR + 4);
        setlinestyle(PS_SOLID, 1);
    } else {
        // ---- 正常飞龙渲染 ----
        int r = m_phase == BossPhase::ENRAGED ? 30 : 24;

        COLORREF body = m_phase == BossPhase::ENRAGED ? RGB(255, 40, 10) : RGB(220, 80, 20);
        // 暗色外圈
        setfillcolor(RGB(60, 20, 5));
        solidcircle(cx, cy, r + 3);
        // 主体
        setfillcolor(body);
        solidcircle(cx, cy, r);
        // 翅膀
        COLORREF wingColor = m_phase == BossPhase::ENRAGED ? RGB(230, 30, 5) : RGB(200, 60, 10);
        setfillcolor(wingColor);
        solidrectangle(cx - 18, cy - 8, cx - 6, cy + 8);
        solidrectangle(cx + 6, cy - 8, cx + 18, cy + 8);
        // 眼睛
        setfillcolor(RGB(255, 255, 0));
        solidcircle(cx - 5, cy - 4, 4);
        solidcircle(cx + 5, cy - 4, 4);
        setfillcolor(RGB(0, 0, 0));
        solidcircle(cx - 5, cy - 4, 2);
        solidcircle(cx + 5, cy - 4, 2);
    }

    // HP 条（始终显示）
    int barW = 80, barH = 6;
    int barX = cx - barW / 2;
    int barY = cy - (m_isBurrowed ? 40 : (m_phase == BossPhase::ENRAGED ? 38 : 32));
    float hpRatio = (float)m_hp / m_maxHP;
    setfillcolor(RGB(40, 40, 40));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    setfillcolor(hpRatio > 0.5f ? RGB(220, 80, 20) : RGB(255, 200, 20));
    solidrectangle(barX, barY, barX + (int)(barW * hpRatio), barY + barH);
}

void LavaDragon::TakeDamage(int damage) {
    if (m_isDead) return;
    if (m_isBurrowed) return;  // 潜地期间无敌
    Boss::TakeDamage(damage);
}

void LavaDragon::CastSkill1() {
    // 扇形火焰喷吐
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    float baseAngle = m_pTarget ? std::atan2(m_pTarget->GetPosition().y - m_position.y,
                                              m_pTarget->GetPosition().x - m_position.x) : 0.0f;
    for (int i = 0; i < m_fireBreathProjectileCount; ++i) {
        float t = m_fireBreathProjectileCount > 1 ? (float)i / (m_fireBreathProjectileCount - 1) : 0.5f;
        float offset = (t - 0.5f) * DegToRad(m_fireBreathSpreadAngle);
        float angle = baseAngle + offset;
        Bullet* b = em.SpawnEntity<Bullet>();
        b->SetPosition(m_position);
        b->SetDirection(angle);
        b->SetSpeed(200.0f + RandomFloat(0, 80));
        b->SetDamage(2);
        b->SetLifetime(2.0f);
        b->SetFaction(BulletFaction::ENEMY);
        b->SetCausesBurning(true);
        b->SetBurnChance(0.4f);
        b->SetBurnDuration(2.0f);
        b->SetColor(0xFF4400);
    }
}

void LavaDragon::CastSkill2() {
    // 潜地追踪
    m_isBurrowed = true;
    m_burrowTimer = 3.0f;
    m_burrowTrailTimer = 0.0f;
    m_burrowTrail.clear();
    if (m_pTarget) m_burrowTargetPos = m_pTarget->GetPosition();
    else m_burrowTargetPos = m_position;
    printf("[LavaDragon] Burrowed! Tracking player...\n");
}
