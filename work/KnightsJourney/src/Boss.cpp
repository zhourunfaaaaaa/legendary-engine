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
#include "../include/Graphics/VisualEffects.h"
#include <graphics.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

void DrawBossShadow(int cx, int cy, int halfW, int halfH) {
    VisualFX::DrawPixelShadow(cx, cy + halfH, halfW, 12);
}

void DrawBossMiniHP(int cx, int cy, int width, int hp, int maxHP, COLORREF fill) {
    if (maxHP <= 0) return;
    float hpRatio = (float)hp / maxHP;
    VisualFX::DrawProgressBar(cx - width / 2, cy, width, 10, hpRatio,
                              fill, RGB(42, 24, 28), RGB(93, 78, 72));
}

void DrawBossEyes(int cx, int cy, COLORREF eyeColor) {
    setfillcolor(eyeColor);
    solidrectangle(cx - 13, cy - 6, cx - 5, cy + 2);
    solidrectangle(cx + 5, cy - 6, cx + 13, cy + 2);
    setfillcolor(RGB(8, 10, 12));
    solidrectangle(cx - 10, cy - 3, cx - 7, cy);
    solidrectangle(cx + 8, cy - 3, cx + 11, cy);
}

Bullet* SpawnBossBullet(EntityManager& em, const Vector2& pos, float angle,
                        float speed, int damage, float lifetime, unsigned int color,
                        float collisionSize = 6.0f) {
    Bullet* b = em.SpawnEntity<Bullet>();
    b->SetPosition(pos);
    b->SetDirection(angle);
    b->SetSpeed(speed);
    b->SetDamage(damage);
    b->SetLifetime(lifetime);
    b->SetFaction(BulletFaction::ENEMY);
    b->SetColor(color);
    b->SetCollisionSize(collisionSize, collisionSize);
    return b;
}

} // namespace

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
    float speedMul = m_phase == BossPhase::ENRAGED ? 1.22f :
                     m_phase == BossPhase::PHASE_2 ? 1.10f : 1.0f;
    m_position += m_moveDirection * m_moveSpeed * speedMul * deltaTime;

    // 自动释放技能
    float cdMul = m_phase == BossPhase::ENRAGED ? 0.76f :
                  m_phase == BossPhase::PHASE_2 ? 0.88f : 1.0f;
    if (CanCastSkill1()) { CastSkill1(); m_skill1Timer = m_skill1Cooldown * cdMul; }
    if (CanCastSkill2()) { CastSkill2(); m_skill2Timer = m_skill2Cooldown * cdMul; }

    // 边界限制
    if (m_position.x < ROOM_BOSS_MARGIN) m_position.x = ROOM_BOSS_MARGIN;
    if (m_position.y < ROOM_BOSS_MARGIN) m_position.y = ROOM_BOSS_MARGIN;
    if (m_position.x > ROOM_WIDTH - ROOM_BOSS_MARGIN) m_position.x = ROOM_WIDTH - ROOM_BOSS_MARGIN;
    if (m_position.y > ROOM_HEIGHT - ROOM_BOSS_MARGIN) m_position.y = ROOM_HEIGHT - ROOM_BOSS_MARGIN;
}

// ============================================================
// Treant - 巨型树人 (Forest Boss)
// ============================================================
Treant::Treant()
    : m_shockwaveProjectileCount(18), m_shockwaveRadius(20.0f)
    , m_shockwaveSpeed(155.0f), m_summonCount(3) {
    m_skill1Cooldown = 4.8f;
    m_skill2Cooldown = 9.2f;
    SetMaxHP(120);
    SetHP(120);
}

void Treant::Update(float deltaTime) { Boss::Update(deltaTime); }

void Treant::Render() {
    if (m_isDead) return;
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    {
        bool enraged = m_phase == BossPhase::ENRAGED;
        COLORREF bark = enraged ? RGB(126, 58, 35) : RGB(91, 62, 36);
        COLORREF leaf = enraged ? RGB(189, 76, 38) : RGB(55, 146, 72);
        COLORREF outline = RGB(19, 24, 18);

        DrawBossShadow(cx, cy, 54, 36);
        VisualFX::DrawPixelRect(cx - 24, cy - 20, cx + 24, cy + 48,
                                bark, outline, 6);
        VisualFX::DrawPixelRect(cx - 44, cy - 64, cx + 44, cy - 14,
                                leaf, outline, 6);
        VisualFX::DrawPixelRect(cx - 31, cy - 86, cx + 31, cy - 45,
                                VisualFX::ScaleColor(leaf, 1.12f), outline, 5);
        VisualFX::DrawPixelRect(cx - 62, cy - 52, cx - 34, cy - 18,
                                VisualFX::ScaleColor(leaf, 0.88f), outline, 5);
        VisualFX::DrawPixelRect(cx + 34, cy - 52, cx + 62, cy - 18,
                                VisualFX::ScaleColor(leaf, 0.88f), outline, 5);

        setfillcolor(VisualFX::ScaleColor(bark, 0.72f));
        solidrectangle(cx - 56, cy + 5, cx - 24, cy + 18);
        solidrectangle(cx + 24, cy + 5, cx + 56, cy + 18);
        setfillcolor(RGB(223, 178, 78));
        solidrectangle(cx - 7, cy + 12, cx + 7, cy + 28);
        DrawBossEyes(cx, cy - 28, enraged ? RGB(255, 210, 64) : RGB(255, 236, 132));
        DrawBossMiniHP(cx, cy - 105, 118, m_hp, m_maxHP,
                       enraged ? RGB(255, 107, 62) : RGB(218, 77, 76));
        return;
    }
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
    // 震荡波：环形压迫 + 对玩家当前位置预判的根刺
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    int ringCount = m_shockwaveProjectileCount;
    if (m_phase == BossPhase::PHASE_2) ringCount += 4;
    if (m_phase == BossPhase::ENRAGED) ringCount += 8;
    float speed = m_shockwaveSpeed + (m_phase == BossPhase::ENRAGED ? 45.0f : 0.0f);

    for (int i = 0; i < ringCount; ++i) {
        float angle = (float)i * 2.0f * PI / ringCount;
        SpawnBossBullet(em, m_position, angle, speed, 3, 3.4f, 0x886644, 7.0f);
    }

    if (m_pTarget) {
        Vector2 lead = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 54.0f;
        Vector2 aim = (lead - m_position).Normalized();
        float baseAngle = std::atan2(aim.y, aim.x);
        int rootCount = m_phase == BossPhase::PHASE_1 ? 3 :
                        m_phase == BossPhase::PHASE_2 ? 5 : 7;
        for (int i = 0; i < rootCount; ++i) {
            float t = rootCount > 1 ? (float)i / (rootCount - 1) : 0.5f;
            float offset = (t - 0.5f) * DegToRad(m_phase == BossPhase::ENRAGED ? 42.0f : 28.0f);
            Bullet* root = SpawnBossBullet(em, m_position, baseAngle + offset,
                                           245.0f, 2, 3.2f, 0x55AA44, 7.0f);
            root->SetCausesSlow(true);
            root->SetSlowDuration(0.75f);
        }
    }
}

void Treant::CastSkill2() {
    // 召唤森林小队：阶段越高，阵容越不单一
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    int count = m_summonCount + (m_phase == BossPhase::ENRAGED ? 2 : (m_phase == BossPhase::PHASE_2 ? 1 : 0));
    for (int i = 0; i < count; ++i) {
        Enemy* add = nullptr;
        int rollMax = m_phase == BossPhase::PHASE_1 ? 1 : 3;
        int roll = RandomInt(0, rollMax);
        if (roll == 0) add = em.SpawnEntity<GoblinMelee>();
        else if (roll == 1) add = em.SpawnEntity<GoblinArcher>();
        else if (roll == 2) add = em.SpawnEntity<ForestWisp>();
        else add = em.SpawnEntity<ChargeBoar>();

        float px = m_position.x + RandomFloat(-96.0f, 96.0f);
        float py = m_position.y + RandomFloat(-96.0f, 96.0f);
        if (px < ROOM_ENTITY_MARGIN) px = ROOM_ENTITY_MARGIN;
        if (py < ROOM_ENTITY_MARGIN) py = ROOM_ENTITY_MARGIN;
        if (px > ROOM_WIDTH - ROOM_ENTITY_MARGIN) px = ROOM_WIDTH - ROOM_ENTITY_MARGIN;
        if (py > ROOM_HEIGHT - ROOM_ENTITY_MARGIN) py = ROOM_HEIGHT - ROOM_ENTITY_MARGIN;
        add->SetPosition(px, py);
        add->SetTarget(GameManager::GetInstance().GetPlayer());
    }
}

// ============================================================
// CrystalCrab - 水晶巨蟹 (Ice Boss)
// ============================================================
CrystalCrab::CrystalCrab()
    : m_isShelled(false), m_shellTimer(0.0f), m_shellHealPerSecond(8.0f)
    , m_laserAngle(0.0f), m_laserRotateSpeed(90.0f), m_laserCount(2) {
    m_skill1Cooldown = 2.9f;
    m_skill2Cooldown = 7.3f;
    SetMaxHP(220);
    SetHP(220);
    SetMoveSpeed(92.0f);
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
    {
        bool enraged = m_phase == BossPhase::ENRAGED;
        COLORREF shell = m_isShelled ? RGB(112, 220, 244) :
                         (enraged ? RGB(54, 128, 232) : RGB(56, 153, 210));
        COLORREF outline = RGB(13, 34, 54);

        DrawBossShadow(cx, cy, 58, 34);
        VisualFX::DrawPixelRect(cx - 54, cy - 10, cx - 26, cy + 24,
                                RGB(49, 113, 168), outline, 5);
        VisualFX::DrawPixelRect(cx + 26, cy - 10, cx + 54, cy + 24,
                                RGB(49, 113, 168), outline, 5);
        VisualFX::DrawPixelRect(cx - 42, cy - 34, cx + 42, cy + 36,
                                shell, outline, 6);
        VisualFX::DrawPixelRect(cx - 28, cy - 49, cx + 28, cy - 26,
                                VisualFX::ScaleColor(shell, 1.18f), outline, 5);

        VisualFX::DrawPixelDiamond(cx - 26, cy - 55, 12, RGB(178, 239, 255), outline);
        VisualFX::DrawPixelDiamond(cx + 26, cy - 55, 12, RGB(178, 239, 255), outline);
        VisualFX::DrawPixelDiamond(cx, cy - 63, 16,
                                  m_isShelled ? RGB(235, 255, 255) : RGB(140, 222, 255),
                                  outline);

        setfillcolor(outline);
        solidrectangle(cx - 67, cy + 2, cx - 48, cy + 12);
        solidrectangle(cx + 48, cy + 2, cx + 67, cy + 12);
        setfillcolor(RGB(242, 83, 94));
        solidrectangle(cx - 16, cy - 8, cx - 7, cy + 1);
        solidrectangle(cx + 7, cy - 8, cx + 16, cy + 1);
        if (m_isShelled) {
            setlinecolor(RGB(228, 255, 255));
            setlinestyle(PS_SOLID, 3);
            rectangle(cx - 50, cy - 42, cx + 50, cy + 42);
            setlinestyle(PS_SOLID, 1);
        }

        DrawBossMiniHP(cx, cy - 78, 118, m_hp, m_maxHP,
                       m_isShelled ? RGB(116, 218, 245) : RGB(70, 140, 230));
        return;
    }
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
    // 交叉激光 + 朝向玩家散射冰刺，后期追加外圈冰晶
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    // 旋转激光
    int laserCount = m_laserCount + (m_phase == BossPhase::ENRAGED ? 1 : 0);
    for (int i = 0; i < laserCount; ++i) {
        float angle = m_laserAngle + (float)i * 2.0f * PI / laserCount;
        for (int j = 0; j < 6; ++j) {
            SpawnBossBullet(em,
                            Vector2(m_position.x + std::cos(angle) * 30.0f,
                                    m_position.y + std::sin(angle) * 30.0f),
                            angle, 220.0f + j * 20.0f, 2, 2.7f, 0x44CCFF, 6.0f);
        }
    }
    m_laserAngle += DegToRad(m_laserRotateSpeed / 4.0f);

    // 额外朝向玩家发射扇形冰刺
    if (m_pTarget) {
        float aimAngle = std::atan2(m_pTarget->GetPosition().y - m_position.y,
                                    m_pTarget->GetPosition().x - m_position.x);
        int burstCount = m_phase == BossPhase::ENRAGED ? 9 : 7;
        float spreadDeg = m_phase == BossPhase::ENRAGED ? 54.0f : 40.0f;
        for (int i = 0; i < burstCount; ++i) {
            float t = (burstCount > 1) ? (float)i / (burstCount - 1) : 0.5f;
            float offset = (t - 0.5f) * DegToRad(spreadDeg);
            float a = aimAngle + offset;
            Bullet* b = SpawnBossBullet(em, m_position, a, 185.0f, 1, 3.3f, 0x88DDFF, 5.0f);
            b->SetCausesSlow(true);
            b->SetSlowDuration(1.5f);
        }
    }

    if (m_phase != BossPhase::PHASE_1) {
        int ringCount = m_phase == BossPhase::ENRAGED ? 18 : 12;
        float start = m_laserAngle * 0.5f;
        for (int i = 0; i < ringCount; ++i) {
            float angle = start + (float)i * 2.0f * PI / ringCount;
            Bullet* shard = SpawnBossBullet(em, m_position, angle,
                                            m_phase == BossPhase::ENRAGED ? 180.0f : 145.0f,
                                            1, 4.2f, 0xB8F4FF, 5.0f);
            shard->SetCausesSlow(true);
            shard->SetSlowDuration(1.0f);
        }
    }
}

void CrystalCrab::CastSkill2() {
    // 缩壳回血，并用冰环把近身输出逼退
    m_isShelled = true;
    m_shellTimer = 3.2f;
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    int shardCount = m_phase == BossPhase::ENRAGED ? 20 : 14;
    for (int i = 0; i < shardCount; ++i) {
        float angle = (float)i * 2.0f * PI / shardCount;
        Bullet* shard = SpawnBossBullet(em, m_position, angle,
                                        175.0f, 2, 3.6f, 0xD6FAFF, 6.0f);
        shard->SetCausesSlow(true);
        shard->SetSlowDuration(1.4f);
    }
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
            int fireCount = m_phase == BossPhase::ENRAGED ? 18 : 12;
            for (int i = 0; i < fireCount; ++i) {
                float angle = (float)i * 2.0f * PI / fireCount;
                Bullet* ember = SpawnBossBullet(em, m_position, angle,
                                                m_phase == BossPhase::ENRAGED ? 255.0f : 215.0f,
                                                2, 4.8f, 0xFF6A24, 7.0f);
                ember->SetCausesBurning(true);
                ember->SetBurnChance(0.35f);
                ember->SetBurnDuration(1.8f);
            }
            m_burrowTrail.clear();
            printf("[LavaDragon] Emerged! Damage dealt in area.\n");
        }
    }

    // 边界限制
    if (m_position.x < ROOM_BOSS_MARGIN) m_position.x = ROOM_BOSS_MARGIN;
    if (m_position.y < ROOM_BOSS_MARGIN) m_position.y = ROOM_BOSS_MARGIN;
    if (m_position.x > ROOM_WIDTH - ROOM_BOSS_MARGIN) m_position.x = ROOM_WIDTH - ROOM_BOSS_MARGIN;
    if (m_position.y > ROOM_HEIGHT - ROOM_BOSS_MARGIN) m_position.y = ROOM_HEIGHT - ROOM_BOSS_MARGIN;

    SyncAABBToPosition();
}

void LavaDragon::Render() {
    if (m_isDead) return;

    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    {
        bool enraged = m_phase == BossPhase::ENRAGED;
        COLORREF body = enraged ? RGB(238, 55, 30) : RGB(203, 79, 35);
        COLORREF outline = RGB(48, 18, 14);

        if (m_isBurrowed) {
            for (size_t i = 0; i < m_burrowTrail.size(); ++i) {
                float t = m_burrowTrail.empty() ? 0.0f : (float)i / (float)m_burrowTrail.size();
                int half = 10 + (int)(t * 14.0f);
                int tx = (int)m_burrowTrail[i].x;
                int ty = (int)m_burrowTrail[i].y;
                VisualFX::DrawPixelRect(tx - half, ty - 6, tx + half, ty + 6,
                                        RGB(166 + (int)(t * 70), 54 + (int)(t * 70), 24),
                                        outline, 3);
            }
            int pulse = 24 + (int)(std::sin(m_burrowTimer * 8.0f) * 5.0f);
            VisualFX::DrawPixelRect(cx - pulse, cy - 16, cx + pulse, cy + 16,
                                    RGB(232, 80, 32), outline, 5);
            VisualFX::DrawPixelRect(cx - pulse / 2, cy - 8, cx + pulse / 2, cy + 8,
                                    RGB(255, 190, 64), RGB(105, 35, 18), 4);
            DrawBossMiniHP(cx, cy - 48, 118, m_hp, m_maxHP, RGB(255, 130, 52));
            return;
        }

        DrawBossShadow(cx, cy, 70, 38);
        for (int i = 0; i < 4; ++i) {
            int ox = (i - 1) * 24;
            int oy = std::abs(i - 1) * 5;
            VisualFX::DrawPixelRect(cx + ox - 22, cy - 22 + oy,
                                    cx + ox + 22, cy + 22 + oy,
                                    i == 1 ? body : VisualFX::ScaleColor(body, 0.84f),
                                    outline, 5);
        }
        VisualFX::DrawPixelRect(cx + 22, cy - 42, cx + 72, cy + 12,
                                body, outline, 6);
        VisualFX::DrawPixelRect(cx - 34, cy - 42, cx - 10, cy - 6,
                                VisualFX::ScaleColor(body, 0.75f), outline, 4);
        VisualFX::DrawPixelRect(cx + 2, cy - 56, cx + 28, cy - 18,
                                VisualFX::ScaleColor(body, 0.82f), outline, 4);
        VisualFX::DrawPixelRect(cx + 58, cy - 68, cx + 72, cy - 44,
                                RGB(255, 211, 91), outline, 4);
        VisualFX::DrawPixelRect(cx + 32, cy - 66, cx + 46, cy - 43,
                                RGB(255, 211, 91), outline, 4);

        setfillcolor(enraged ? RGB(255, 232, 92) : RGB(255, 212, 72));
        solidrectangle(cx + 51, cy - 22, cx + 61, cy - 13);
        setfillcolor(RGB(12, 10, 8));
        solidrectangle(cx + 55, cy - 19, cx + 59, cy - 16);
        VisualFX::DrawPixelProjectile(cx + 80, cy - 4, 1.0f, 0.0f,
                                      24, 6, RGB(255, 154, 42), outline);
        DrawBossMiniHP(cx, cy - 88, 132, m_hp, m_maxHP,
                       enraged ? RGB(255, 80, 44) : RGB(232, 94, 44));
        return;
    }

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
    // 扇形火焰喷吐；后期从房间边缘追加横切火球
    EntityManager& em = GameManager::GetInstance().GetEntityManager();
    float baseAngle = m_pTarget ? std::atan2(m_pTarget->GetPosition().y - m_position.y,
                                              m_pTarget->GetPosition().x - m_position.x) : 0.0f;
    int fireCount = m_fireBreathProjectileCount +
                    (m_phase == BossPhase::ENRAGED ? 10 : (m_phase == BossPhase::PHASE_2 ? 5 : 0));
    float spread = m_fireBreathSpreadAngle +
                   (m_phase == BossPhase::ENRAGED ? 22.0f : (m_phase == BossPhase::PHASE_2 ? 12.0f : 0.0f));
    for (int i = 0; i < fireCount; ++i) {
        float t = fireCount > 1 ? (float)i / (fireCount - 1) : 0.5f;
        float offset = (t - 0.5f) * DegToRad(spread);
        float angle = baseAngle + offset;
        Bullet* b = SpawnBossBullet(em, m_position, angle,
                                    215.0f + RandomFloat(0.0f, 95.0f),
                                    2, 4.6f, 0xFF4400, 7.0f);
        b->SetCausesBurning(true);
        b->SetBurnChance(0.4f);
        b->SetBurnDuration(2.0f);
    }

    if (m_phase != BossPhase::PHASE_1 && m_pTarget) {
        int flankCount = m_phase == BossPhase::ENRAGED ? 8 : 5;
        Vector2 target = m_pTarget->GetPosition() + m_pTarget->GetMoveDirection() * 58.0f;
        for (int i = 0; i < flankCount; ++i) {
            int edge = RandomInt(0, 3);
            Vector2 origin;
            switch (edge) {
                case 0: origin = Vector2(RandomFloat(80.0f, ROOM_WIDTH - 80.0f), 70.0f); break;
                case 1: origin = Vector2(RandomFloat(80.0f, ROOM_WIDTH - 80.0f), ROOM_HEIGHT - 70.0f); break;
                case 2: origin = Vector2(70.0f, RandomFloat(80.0f, ROOM_HEIGHT - 80.0f)); break;
                default: origin = Vector2(ROOM_WIDTH - 70.0f, RandomFloat(80.0f, ROOM_HEIGHT - 80.0f)); break;
            }
            Vector2 dir = (target - origin).Normalized();
            float angle = std::atan2(dir.y, dir.x);
            Bullet* b = SpawnBossBullet(em, origin, angle,
                                        m_phase == BossPhase::ENRAGED ? 285.0f : 250.0f,
                                        2, 5.8f, 0xFF9E32, 7.0f);
            b->SetCausesBurning(true);
            b->SetBurnChance(0.35f);
            b->SetBurnDuration(1.8f);
        }
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
