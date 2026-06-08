#include "Player.h"
#include "GameManager.h"
#include "Input.h"
#include "Audio.h"

Player::Player(float x, float y)
    : m_x(x), m_y(y)
    , m_size(PLAYER_SIZE)
    , m_speed(PLAYER_SPEED)
    , m_hp(PLAYER_HP), m_maxHp(PLAYER_HP)
    , m_energy(0.0f), m_maxEnergy(PLAYER_MAX_ENERGY)
    , m_invincible(0.0f)
    , m_weaponType(WeaponType::BEAM)
    , m_shootTimer(0.0f), m_shootInterval(PLAYER_SHOOT_INTERVAL)
    , m_recoilX(0.0f), m_recoilY(0.0f)
    , m_engineFlame(0.0f), m_glowTimer(0.0f)
{
}

void Player::Update(float dt) {
    // 引擎动画
    m_engineFlame += dt * 10.0f;
    // 发光衰减
    m_glowTimer = (std::max)(0.0f, m_glowTimer - dt);

    // 拖拽移动
    Vector2 drag = Input::GetInstance().GetDragDelta();
    m_x += drag.x;
    m_y += drag.y;

    // 键盘移动
    Vector2 kb = Input::GetInstance().GetKeyboardDir();
    m_x += kb.x * m_speed * dt;
    m_y += kb.y * m_speed * dt;

    // 边界限制
    m_x = ClampFloat(m_x, m_size, WINDOW_WIDTH - m_size);
    m_y = ClampFloat(m_y, m_size, WINDOW_HEIGHT - m_size);

    // 后坐力衰减
    m_recoilX *= std::pow(0.01f, dt);
    m_recoilY *= std::pow(0.01f, dt);

    // 无敌时间
    m_invincible = (std::max)(0.0f, m_invincible - dt);

    // 射击冷却
    m_shootTimer -= dt;
}

bool Player::CanShoot() const {
    return m_shootTimer <= 0.0f;
}

void Player::Shoot() {
    m_shootTimer = m_shootInterval;
    // 后座力
    m_recoilY = (m_weaponType == WeaponType::PULSE) ? -4.0f : -1.0f;
    m_recoilX = (RandomFloat(0.0f, 1.0f) - 0.5f) * 1.0f;
}

bool Player::TakeDamage(int amount) {
    if (m_invincible > 0.0f) return false;
    m_hp -= amount;
    m_invincible = INVINCIBLE_TIME;

    GameManager& gm = GameManager::GetInstance();
    gm.AddScreenShake(5.0f, 0.15f);
    gm.AddScreenFlash(0.2f);
    Audio::GetInstance().PlayDamage();
    return true;
}

void Player::AddEnergy(int amount) {
    m_energy = (std::min)(m_maxEnergy, m_energy + (float)amount);
}

bool Player::UseUltimate() {
    if (m_energy < m_maxEnergy) return false;
    m_energy = 0.0f;
    return true;
}

float Player::GetHitRadius() const {
    // 阶段3缩小判定
    GameManager& gm = GameManager::GetInstance();
    return (gm.GetCurrentStage() >= 2) ? 8.0f : 12.0f;
}
