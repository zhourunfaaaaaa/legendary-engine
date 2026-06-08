#include "Enemy.h"
#include "GameManager.h"

Enemy::Enemy(float x, float y, EnemyType type)
    : m_x(x), m_y(y)
    , m_alive(true)
    , m_type(type)
    , m_flashTimer(0.0f)
    , m_spiralAngle(0.0f), m_phase(0.0f)
    , m_shieldActive(false), m_shieldMax(0.0f), m_shieldDR(0.0f)
    , m_splitCount(0), m_hasSplit(false)
    , m_warningLine(false), m_warnTimer(0.0f), m_sniperAngle(0.0f)
{
    GameManager& gm = GameManager::GetInstance();
    float diffScale = 1.0f; // 在构造函数中暂用默认值，后续可动态调整

    switch (type) {
        case EnemyType::SMALL:
            m_hp = 30; m_maxHp = 30;
            m_size = 15.0f; m_speed = 100.0f;
            m_scoreValue = 100;
            m_color = RGB(255, 0, 255); // #f0f
            m_shootInterval = 2.0f;
            m_shootTimer = RandomFloat(0.0f, 2.0f);
            m_pattern = ShootPattern::SINGLE;
            break;
        case EnemyType::NORMAL:
            m_hp = 80; m_maxHp = 80;
            m_size = 25.0f; m_speed = 60.0f;
            m_scoreValue = 250;
            m_color = RGB(255, 128, 0); // #f80
            m_shootInterval = 1.5f;
            m_shootTimer = RandomFloat(0.0f, 1.5f);
            m_pattern = ShootPattern::SPREAD;
            break;
        case EnemyType::LARGE:
            m_hp = 300; m_maxHp = 300;
            m_size = 45.0f; m_speed = 35.0f;
            m_scoreValue = 1000;
            m_color = RGB(255, 0, 0); // #f00
            m_shootInterval = 0.8f;
            m_shootTimer = 0.5f;
            m_pattern = ShootPattern::SPIRAL;
            break;
        case EnemyType::FAST:
            {
                int wave = gm.GetWave();
                float fs = 1.0f + (std::max)(0, wave - 1) * 0.08f;
                m_hp = (int)(25.0f * fs); m_maxHp = m_hp;
                m_size = 12.0f;
                m_speed = (std::min)(180.0f + wave * 8.0f, 300.0f);
                m_scoreValue = 120;
                m_color = RGB(0, 255, 0); // #0f0
                m_shootInterval = (std::max)(0.6f, 1.6f - wave * 0.05f);
                m_shootTimer = RandomFloat(0.0f, m_shootInterval);
                m_pattern = ShootPattern::SINGLE;
                m_wobbleAmp = 10.0f;
            }
            break;
        case EnemyType::SHIELD:
            {
                int wave = gm.GetWave();
                float shs = 1.0f + (std::max)(0, wave - 1) * 0.08f;
                m_hp = (int)(200.0f * shs); m_maxHp = m_hp;
                m_size = 35.0f; m_speed = 40.0f;
                m_scoreValue = 600;
                m_color = RGB(0, 136, 255); // #08f
                m_shootInterval = (std::max)(0.7f, 2.0f - wave * 0.06f);
                m_shootTimer = RandomFloat(0.0f, m_shootInterval);
                m_pattern = ShootPattern::SPREAD;
                m_shieldActive = true;
                m_shieldMax = (float)m_hp;
                m_shieldDR = (gm.GetCurrentStage() >= 2) ? 0.50f : 0.30f;
            }
            break;
        case EnemyType::SPLITTER:
            {
                int wave = gm.GetWave();
                float sps = 1.0f + (std::max)(0, wave - 1) * 0.08f;
                m_hp = (int)(120.0f * sps); m_maxHp = m_hp;
                m_size = 22.0f; m_speed = 50.0f;
                m_scoreValue = 400;
                m_color = RGB(255, 170, 0); // #fa0
                m_shootInterval = (std::max)(0.8f, 1.8f - wave * 0.05f);
                m_shootTimer = RandomFloat(0.0f, m_shootInterval);
                m_pattern = ShootPattern::SPREAD;
                m_splitCount = (std::min)(3 + (wave - 8) / 2, 6);
                if (m_splitCount < 2) m_splitCount = 2;
            }
            break;
        case EnemyType::SNIPER:
            {
                int wave = gm.GetWave();
                float sns = 1.0f + (std::max)(0, wave - 1) * 0.08f;
                m_hp = (int)(80.0f * sns); m_maxHp = m_hp;
                m_size = 18.0f; m_speed = 30.0f;
                m_scoreValue = 350;
                m_color = RGB(255, 0, 136); // #f08
                m_shootInterval = (std::max)(1.0f, 2.5f - wave * 0.07f);
                m_shootTimer = m_shootInterval * 0.5f;
                m_pattern = ShootPattern::SNIPER;
            }
            break;
    }

    m_wobble = RandomFloat(0.0f, PI * 2.0f);
    m_wobbleSpeed = 1.0f + RandomFloat(0.0f, 2.0f);
    m_wobbleAmp = (type != EnemyType::FAST) ? 30.0f + RandomFloat(0.0f, 40.0f) : m_wobbleAmp;
}

void Enemy::Update(float dt) {
    m_flashTimer = (std::max)(0.0f, m_flashTimer - dt);

    // 向下移动 + 摇摆
    m_y += m_speed * dt;
    m_wobble += dt * m_wobbleSpeed;
    m_x += std::sin(m_wobble) * m_wobbleAmp * dt;

    // 出界销毁
    if (m_y > WINDOW_HEIGHT + 100) {
        m_alive = false;
    }
}

Vector2 Enemy::GetShootPos() const {
    return Vector2(
        m_x + (RandomFloat(0.0f, 1.0f) - 0.5f) * m_size * 0.6f,
        m_y + m_size * 0.5f
    );
}
