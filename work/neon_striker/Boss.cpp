#include "Boss.h"
#include "GameManager.h"

Boss::Boss(float x, float y, int hp, bool isInfinite)
    : m_x(x), m_y(y)
    , m_size(isInfinite ? 100.0f : 120.0f)
    , m_hp(hp), m_maxHp(hp)
    , m_alive(true), m_isInfinite(isInfinite)
    , m_bossPhase(0)
    , m_flashTimer(0.0f), m_invulnTimer(0.0f)
    , m_moveDir(1)
    , m_ringBurstTimer(2.0f),  m_ringBurstCD(4.5f)
    , m_aimedFanTimer(1.0f),   m_aimedFanCD(2.8f)
    , m_warnBeamTimer(6.0f),   m_warnBeamCD(10.0f)
    , m_laserOrbTimer(6.0f),   m_laserOrbCD(9.0f)
    , m_bounceTimer(3.0f),     m_bounceCD(6.0f)
    , m_hasRingBurst(true), m_hasAimedFan(true)
    , m_hasWarnBeam(false), m_hasBounce(false), m_hasLaserOrb(false)
    , m_warnBeamActive(false), m_warnBeamPhase(0)
    , m_warnBeamTimer2(0.0f), m_warnBeamAngle(0.0f)
    , m_phase3SpeedMult(0.75f)
    , m_spiralAngle(0.0f)
{
}

void Boss::Update(float dt) {
    GameManager& gm = GameManager::GetInstance();

    m_flashTimer = (std::max)(0.0f, m_flashTimer - dt);
    m_invulnTimer = (std::max)(0.0f, m_invulnTimer - dt);

    // 移动
    m_x += m_moveDir * BOSS_SPEED * dt;
    m_y = 80.0f + std::sin(gm.GetGameTime() * 0.5f) * 40.0f;
    if (m_x > WINDOW_WIDTH - m_size) m_moveDir = -1;
    if (m_x < m_size) m_moveDir = 1;

    // 攻击冷却
    m_ringBurstTimer -= dt;
    m_aimedFanTimer -= dt;
    m_warnBeamTimer -= dt;
    m_laserOrbTimer -= dt;
    m_bounceTimer -= dt;

    // 预警激光状态机
    if (m_warnBeamActive) {
        m_warnBeamTimer2 -= dt;
        if (m_warnBeamPhase == 1 && m_warnBeamTimer2 <= 0.0f) {
            m_warnBeamPhase = 2;
            m_warnBeamTimer2 = (m_bossPhase >= 2) ? 0.8f : 0.6f;
        } else if (m_warnBeamPhase == 2 && m_warnBeamTimer2 <= 0.0f) {
            m_warnBeamActive = false;
            m_warnBeamPhase = 0;
        }
    }

    // 更新阶段 (基于血量%)
    float hpPct = (float)m_hp / (float)m_maxHp;
    int newPhase = (hpPct < 0.15f) ? 2 : (hpPct < 0.50f) ? 1 : 0;
    if (newPhase > m_bossPhase) {
        m_bossPhase = newPhase;
        // 阶段2：解锁Bounce + LaserOrb
        if (m_bossPhase >= 1 && m_isInfinite) {
            m_hasBounce = true;
            m_hasLaserOrb = true;
        }
        // 阶段3：解锁WarnBeam
        if (m_bossPhase >= 2) {
            m_hasWarnBeam = true;
        }
        // 阶段切换无敌
        m_invulnTimer = 0.5f;
    }
}

Vector2 Boss::GetShootPos() const {
    return Vector2(
        m_x + (RandomFloat(0.0f, 1.0f) - 0.5f) * m_size * 0.3f,
        m_y + m_size * 0.4f
    );
}
