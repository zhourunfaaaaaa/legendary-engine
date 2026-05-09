// ============================================================
// Player.cpp - 玩家实现（3 职业、输入、渲染、技能、武器）
// ============================================================

#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Weapon/WeaponTypes.h"
#include "../include/System/EntityManager.h"
#include "../include/System/BuffManager.h"
#include "../include/Core/GameManager.h"

#include <graphics.h>
#include <windows.h>
#include <cmath>
#include <cstdio>

// ============================================================
// 构造 / 析构
// ============================================================
Player::Player()
    : m_profession(Profession::KNIGHT)
    , m_currentWeapon(nullptr)
    , m_weaponCount(0)
    , m_skillCooldown(10.0f)
    , m_skillCooldownRemaining(0.0f)
    , m_skillCooldownModifier(1.0f)
    , m_fullFireActive(false)
    , m_fullFireTimer(0.0f)
    , m_isRolling(false)
    , m_rollInvincible(false)
    , m_firstShotAfterRoll(false)
    , m_rollDirection(0.0f, 0.0f)
    , m_rollTimer(0.0f)
    , m_rollDistance(150.0f)
    , m_lightningVfxTimer(0.0f)
    , m_aimDirection(1.0f, 0.0f)
    , m_aimAngle(0.0f)
    , m_keyW(false), m_keyA(false), m_keyS(false), m_keyD(false)
    , m_keySpace(false)
    , m_mouseLeft(false)
    , m_mouseLeftPrev(false)
    , m_fireLocked(false)
    , m_gold(0)
    , m_hasReviveCharm(false)
    , m_damageInvincibleTimer(0.0f)
    , m_pendingWeaponType(WeaponType::ASSAULT_RIFLE) {
    SetRenderLayer(10);          // 玩家渲染层级最高
    SetCollisionSize(16.0f, 16.0f);  // 碰撞盒半径 16

    // 初始武器槽全部为空
    for (int i = 0; i < MAX_WEAPON_SLOTS; ++i) {
        m_weapons[i] = nullptr;
        m_keyWeaponPrev[i] = false;
    }
}

Player::~Player() {
}

// ============================================================
// 职业初始化
// ============================================================
void Player::InitProfession(Profession prof) {
    m_profession = prof;

    switch (prof) {
        case Profession::KNIGHT:
            SetMaxHP(7);
            SetHP(7);
            SetMaxMP(200);
            SetMP(200);
            SetMoveSpeed(220.0f);
            SetArmor(1);                     // 骑士有 1 点护甲
            SetMaxShield(4);                 // 骑士护盾
            SetShield(4);
            m_skillCooldown = 12.0f;         // 火力全开冷却 12 秒
            break;

        case Profession::ROGUE:
            SetMaxHP(5);
            SetHP(5);
            SetMaxMP(200);
            SetMP(200);
            SetMoveSpeed(270.0f);            // 快速
            SetArmor(0);
            SetMaxShield(2);                 // 游侠护盾
            SetShield(2);
            m_skillCooldown = 3.0f;          // 翻滚冷却 3 秒
            break;

        case Profession::MAGE:
            SetMaxHP(4);
            SetHP(4);
            SetMaxMP(250);
            SetMP(250);
            SetMoveSpeed(180.0f);            // 慢速
            SetArmor(0);
            SetMaxShield(3);                 // 法师护盾
            SetShield(3);
            m_skillCooldown = 10.0f;         // 奥术闪电冷却 10 秒
            break;
    }

    m_skillCooldownRemaining = 0.0f;
    printf("[Player] Profession initialized: %s | HP=%d MP=%d Speed=%.0f\n",
           (prof == Profession::KNIGHT ? "Knight" :
            prof == Profession::ROGUE ? "Rogue" : "Mage"),
           m_maxHP, m_maxMP, m_moveSpeed);
}

// ============================================================
// 每帧更新
// ============================================================
void Player::Update(float deltaTime) {
    if (m_isDead) return;

    // 1. 更新状态效果（燃烧/冰冻/晕眩/无敌）
    UpdateStatusEffects(deltaTime);

    // 2. 受伤无敌帧
    if (m_damageInvincibleTimer > 0.0f) {
        m_damageInvincibleTimer -= deltaTime;
    }

    // 3. 技能冷却
    UpdateSkillCooldown(deltaTime);

    // 3.5. 连锁闪电特效计时
    if (m_lightningVfxTimer > 0.0f) {
        m_lightningVfxTimer -= deltaTime;
        if (m_lightningVfxTimer <= 0.0f) {
            m_lightningChainPositions.clear();
        }
    }

    // 4. 骑士技能计时
    if (m_fullFireActive) {
        UpdateFullFire(deltaTime);
    }

    // 5. 游侠技能计时
    if (m_isRolling) {
        UpdateRoll(deltaTime);
    }

    // 6. 应用移动（若未晕眩且未在翻滚中）
    if (!m_isStunned && !m_isRolling) {
        // 冰冻减速效果在 UpdateStatusEffects 中已通过 m_moveSpeed 体现
        Vector2 movement = m_moveDirection * m_moveSpeed * deltaTime;
        m_position += movement;
    }

    // 7. 边界限制（房间边界 + 玩家半径）
    const float margin = 18.0f;  // 玩家碰撞半径 + 余量
    if (m_position.x < margin) m_position.x = margin;
    if (m_position.y < margin) m_position.y = margin;
    if (m_position.x > ROOM_WIDTH - margin)  m_position.x = ROOM_WIDTH - margin;
    if (m_position.y > ROOM_HEIGHT - margin) m_position.y = ROOM_HEIGHT - margin;

    // 8. 同步碰撞盒
    SyncAABBToPosition();

    // 9. 武器冷却更新
    if (m_currentWeapon) {
        m_currentWeapon->UpdateCooldown(deltaTime);
    }
}

// ============================================================
// 渲染 - 蓝色圆圈 + 朝向线段
// ============================================================
void Player::Render() {
    if (m_isDead) return;

    int cx = static_cast<int>(m_position.x);
    int cy = static_cast<int>(m_position.y);
    const int radius = 16;
    const int lineLen = 24;

    // 受伤闪烁效果（无敌帧期间闪烁）
    if (m_damageInvincibleTimer > 0.0f) {
        int blink = static_cast<int>(m_damageInvincibleTimer * 20) % 2;
        if (blink == 0) return;  // 隔帧不绘制，形成闪烁
    }

    // ---- 玩家身体（蓝色圆） ----
    // 外圈描边（深蓝色）
    setfillcolor(RGB(20, 40, 120));
    solidcircle(cx, cy, radius + 2);

    // 主体填充
    COLORREF bodyColor;
    switch (m_profession) {
        case Profession::KNIGHT: bodyColor = RGB(60, 120, 255); break;   // 亮蓝
        case Profession::ROGUE:  bodyColor = RGB(60, 200, 120); break;   // 亮绿
        case Profession::MAGE:   bodyColor = RGB(180, 80, 255); break;   // 紫色
        default:                 bodyColor = RGB(60, 120, 255); break;
    }
    setfillcolor(bodyColor);
    solidcircle(cx, cy, radius);

    // 中心高光
    setfillcolor(RGB(140, 180, 255));
    solidcircle(cx - 3, cy - 3, radius / 3);

    // ---- 朝向线段（黄色） ----
    int tipX = cx + static_cast<int>(m_aimDirection.x * lineLen);
    int tipY = cy + static_cast<int>(m_aimDirection.y * lineLen);

    setlinestyle(PS_SOLID, 3);
    setlinecolor(RGB(255, 220, 60));
    line(cx, cy, tipX, tipY);

    // 准星端点小圆
    setfillcolor(RGB(255, 240, 100));
    solidcircle(tipX, tipY, 3);

    // 连锁闪电特效
    if (m_lightningVfxTimer > 0.0f && m_lightningChainPositions.size() >= 2) {
        for (size_t i = 0; i < m_lightningChainPositions.size() - 1; ++i) {
            int x1 = (int)m_lightningChainPositions[i].x;
            int y1 = (int)m_lightningChainPositions[i].y;
            int x2 = (int)m_lightningChainPositions[i + 1].x;
            int y2 = (int)m_lightningChainPositions[i + 1].y;
            // 亮青色粗线 + 白色细线叠加
            setlinestyle(PS_SOLID, 3);
            setlinecolor(RGB(0, 200, 255));
            line(x1, y1, x2, y2);
            setlinestyle(PS_SOLID, 1);
            setlinecolor(RGB(200, 255, 255));
            line(x1, y1, x2, y2);
        }
    }

    setlinestyle(PS_SOLID, 1);
}

// ============================================================
// 键盘输入 - GetAsyncKeyState WASD
// ============================================================
void Player::ProcessKeyboardInput(float deltaTime) {
    if (m_isDead) return;
    if (m_isStunned) return;

    // WASD 移动
    m_keyW = (GetAsyncKeyState('W') & 0x8000) != 0;
    m_keyA = (GetAsyncKeyState('A') & 0x8000) != 0;
    m_keyS = (GetAsyncKeyState('S') & 0x8000) != 0;
    m_keyD = (GetAsyncKeyState('D') & 0x8000) != 0;

    // 也支持方向键
    bool up    = m_keyW    || (GetAsyncKeyState(VK_UP)    & 0x8000);
    bool down  = m_keyS    || (GetAsyncKeyState(VK_DOWN)  & 0x8000);
    bool left  = m_keyA    || (GetAsyncKeyState(VK_LEFT)  & 0x8000);
    bool right = m_keyD    || (GetAsyncKeyState(VK_RIGHT) & 0x8000);

    // 计算移动方向向量
    float dirX = 0.0f, dirY = 0.0f;
    if (right) dirX += 1.0f;
    if (left)  dirX -= 1.0f;
    if (down)  dirY += 1.0f;
    if (up)    dirY -= 1.0f;

    // 归一化（防止斜向移动速度变快）
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.001f) {
        m_moveDirection.x = dirX / length;
        m_moveDirection.y = dirY / length;
    } else {
        m_moveDirection.x = 0.0f;
        m_moveDirection.y = 0.0f;
    }

    // 空格 - 技能
    m_keySpace = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    if (m_keySpace && IsSkillReady()) {
        UseSkill();
    }

    // 数字键切换武器（上升沿触发，防止按住时重复切换）
    for (int i = 0; i < MAX_WEAPON_SLOTS; ++i) {
        int vk = '1' + i;
        bool keyNow = (GetAsyncKeyState(vk) & 0x8000) != 0;
        if (keyNow && !m_keyWeaponPrev[i]) {
            SwitchWeapon(i);
        }
        m_keyWeaponPrev[i] = keyNow;
    }
}

// ============================================================
// 鼠标输入 - 瞄准方向
// ============================================================
void Player::ProcessMouseInput() {
    if (m_isDead) return;

    // 使用 Windows API 获取当前鼠标位置（非阻塞），转换为窗口坐标
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetHWnd(), &pt);
    int mouseX = pt.x;
    int mouseY = pt.y;

    // 计算朝向（从玩家指向鼠标）
    float dx = static_cast<float>(mouseX) - m_position.x;
    float dy = static_cast<float>(mouseY) - m_position.y;
    float len = std::sqrt(dx * dx + dy * dy);

    if (len > 1.0f) {
        m_aimDirection.x = dx / len;
        m_aimDirection.y = dy / len;
        m_aimAngle = std::atan2(dy, dx);
    }

    // 左键开火（防状态切换穿透）
    m_mouseLeftPrev = m_mouseLeft;
    m_mouseLeft = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    bool inputBlocked = GameManager::GetInstance().GetStateDelayTimer() > 0.0f;

    // 输入封锁期间按下的鼠标，解锁前不能用于开火
    if (inputBlocked && m_mouseLeft) {
        m_fireLocked = true;
    }
    if (!m_mouseLeft) {
        m_fireLocked = false;
    }

    // 按住左键持续开火（由武器冷却控制实际射速）
    if (m_mouseLeft && !m_fireLocked && !inputBlocked) {
        FireWeapon();
    }
}

// ============================================================
// 武器系统
// ============================================================
bool Player::EquipWeapon(WeaponType type) {
    std::unique_ptr<Weapon> newWeapon;

    switch (type) {
        case WeaponType::ASSAULT_RIFLE:   newWeapon = std::unique_ptr<AssaultRifle>(new AssaultRifle());    break;
        case WeaponType::SHOTGUN:         newWeapon = std::unique_ptr<Shotgun>(new Shotgun());         break;
        case WeaponType::SNIPER_RIFLE:    newWeapon = std::unique_ptr<SniperRifle>(new SniperRifle());     break;
        case WeaponType::ROCKET_LAUNCHER: newWeapon = std::unique_ptr<RocketLauncher>(new RocketLauncher());  break;
        case WeaponType::FLAME_THROWER:   newWeapon = std::unique_ptr<FlameThrower>(new FlameThrower());    break;
        case WeaponType::MAGIC_STAFF:     newWeapon = std::unique_ptr<MagicStaff>(new MagicStaff());      break;
        case WeaponType::REBOUND_CROSSBOW:newWeapon = std::unique_ptr<ReboundCrossbow>(new ReboundCrossbow()); break;
        case WeaponType::VAMPIRE_CODEX:   newWeapon = std::unique_ptr<VampireCodex>(new VampireCodex());    break;
        default: return false;
    }

    return AddWeapon(std::move(newWeapon));
}

bool Player::AddWeapon(std::unique_ptr<Weapon> weapon) {
    if (!weapon) return false;

    WeaponType newType = weapon->GetType();

    // 如果新武器与某个槽位的武器类型相同，自动放入另一个槽位（同种武器双持）
    for (int i = 0; i < MAX_WEAPON_SLOTS; ++i) {
        if (m_weapons[i] && m_weapons[i]->GetType() == newType) {
            int otherSlot = (i == 0) ? 1 : 0;
            printf("[Player] Same weapon type detected in slot %d, equipping to slot %d\n", i, otherSlot);
            m_weapons[otherSlot] = std::move(weapon);
            m_currentWeapon = m_weapons[otherSlot].get();
            if (m_weaponCount < MAX_WEAPON_SLOTS) m_weaponCount++;
            printf("[Player] Weapon equipped in slot %d: %s\n", otherSlot, m_weapons[otherSlot]->GetName());
            return true;
        }
    }

    // 查找空槽位
    for (int i = 0; i < MAX_WEAPON_SLOTS; ++i) {
        if (!m_weapons[i]) {
            m_weapons[i] = std::move(weapon);
            m_weaponCount++;
            if (!m_currentWeapon) {
                m_currentWeapon = m_weapons[i].get();
            }
            printf("[Player] Weapon equipped in slot %d: %s\n", i, m_weapons[i]->GetName());
            return true;
        }
    }

    // 槽位已满，暂存为待替换武器，触发选择
    m_pendingWeapon = std::move(weapon);
    m_pendingWeaponType = m_pendingWeapon->GetType();
    printf("[Player] Weapon slots full, pending: %s\n", m_pendingWeapon->GetName());
    return false;  // 返回 false 让调用者触发 WEAPON_SELECT
}

void Player::ReplaceWeapon(int slot, std::unique_ptr<Weapon> newWeapon) {
    if (slot < 0 || slot >= MAX_WEAPON_SLOTS) return;
    if (!newWeapon) return;

    printf("[Player] Replacing weapon in slot %d: %s -> %s\n",
           slot, m_weapons[slot] ? m_weapons[slot]->GetName() : "empty",
           newWeapon->GetName());

    m_weapons[slot] = std::move(newWeapon);
    m_currentWeapon = m_weapons[slot].get();
}

void Player::SwitchWeapon(int slot) {
    if (slot < 0 || slot >= MAX_WEAPON_SLOTS) return;
    if (!m_weapons[slot]) return;
    m_currentWeapon = m_weapons[slot].get();
    printf("[Player] Switched to weapon slot %d: %s\n", slot, m_currentWeapon->GetName());
}

void Player::FireWeapon() {
    if (!m_currentWeapon) return;
    if (!m_currentWeapon->CanFire(this)) return;

    // 获取 EntityManager（通过全局单例）
    EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();

    // 应用射击精度偏移（若武器有散射角度）
    Vector2 dir = m_aimDirection;

    // 游侠翻滚后第一击必暴击
    if (m_firstShotAfterRoll) {
        // 暴击标记由武器处理（在 Fire 中根据 IsCritical 判定的子弹创建）
        ConsumeFirstShotAfterRoll();
    }

    m_currentWeapon->Fire(m_position, dir, BulletFaction::PLAYER, entityMgr);

    // 消耗 MP（火力全开期间双倍耗蓝）
    int mpCost = m_currentWeapon->GetMPCost();
    if (mpCost > 0) {
        if (m_fullFireActive) {
            mpCost *= 2;  // 双倍射速 = 双倍耗蓝
        }
        SetMP(m_mp - mpCost);
    }
}

// ============================================================
// 职业技能
// ============================================================
void Player::UseSkill() {
    if (!IsSkillReady()) return;

    m_skillCooldownRemaining = m_skillCooldown * m_skillCooldownModifier;

    switch (m_profession) {
        case Profession::KNIGHT: {
            // 【火力全开】5秒内射速翻倍，不消耗MP
            m_fullFireActive = true;
            m_fullFireTimer = 5.0f;
            if (m_currentWeapon) {
                m_currentWeapon->SetFireRateMultiplier(2.0f);
            }
            printf("[Player] Knight Skill: Full Fire activated! (5s)\n");
            break;
        }

        case Profession::ROGUE: {
            // 【战术翻滚】向移动方向突进（无移动输入时默认瞄准方向）
            m_isRolling = true;
            m_rollInvincible = true;
            m_rollTimer = 0.0f;
            // 优先朝键盘移动方向翻滚，没按方向键则朝鼠标方向
            if (m_moveDirection.Length() > 0.1f) {
                m_rollDirection = m_moveDirection;
            } else {
                m_rollDirection = m_aimDirection;
            }
            m_firstShotAfterRoll = true;
            // 翻滚距离和时间
            m_rollDistance = 150.0f;
            // 使用 SetInvincibleDuration 设置无敌计时器，避免被 UpdateStatusEffects 立即清除
            SetInvincibleDuration(0.35f);
            printf("[Player] Rogue Skill: Tactical Roll!\n");
            break;
        }

        case Profession::MAGE: {
            // 【连锁闪电】15点伤害+2秒眩晕主目标，连锁至多3个附近敌人各8点伤害
            EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
            Enemy* primaryTarget = entityMgr.FindNearestEnemy(m_position, 500.0f);
            if (primaryTarget) {
                primaryTarget->TakeDamage(15);
                primaryTarget->ApplyStun(2.0f);

                // 存储闪电特效位置
                m_lightningChainPositions.clear();
                m_lightningChainPositions.push_back(m_position);  // 起始点：玩家位置
                m_lightningChainPositions.push_back(primaryTarget->GetPosition());
                m_lightningVfxTimer = 0.4f;

                int chained = 0;
                Vector2 chainOrigin = primaryTarget->GetPosition();
                std::vector<Enemy*> hitList;
                hitList.push_back(primaryTarget);

                for (int c = 0; c < 3; ++c) {
                    Enemy* nearest = nullptr;
                    float minDist = 200.0f;
                    const auto& entities = entityMgr.GetAllEntities();
                    for (const auto& e : entities) {
                        if (!e || !e->IsActive()) continue;
                        Enemy* enemy = dynamic_cast<Enemy*>(e.get());
                        if (!enemy || enemy->IsDead()) continue;
                        bool alreadyHit = false;
                        for (auto* h : hitList) { if (h == enemy) { alreadyHit = true; break; } }
                        if (alreadyHit) continue;
                        float d = Vector2::Distance(chainOrigin, enemy->GetPosition());
                        if (d < minDist) { minDist = d; nearest = enemy; }
                    }
                    if (!nearest) break;
                    nearest->TakeDamage(8);
                    hitList.push_back(nearest);
                    m_lightningChainPositions.push_back(nearest->GetPosition());
                    chainOrigin = nearest->GetPosition();
                    chained++;
                }
                printf("[Player] Mage Skill: Chain Lightning! 1 primary + %d chains\n", chained);
            } else {
                m_skillCooldownRemaining = 0.0f;
                printf("[Player] Mage Skill: No target in range, cooldown refunded.\n");
            }
            break;
        }
    }
}

void Player::ReduceSkillCooldown(float percent) {
    // CD缩减 buff 调用（percent 为缩减比例，如 0.4 表示减少 40%）
    m_skillCooldownModifier = 1.0f - percent;
    if (m_skillCooldownModifier < 0.1f) m_skillCooldownModifier = 0.1f;
    printf("[Player] Skill cooldown reduced by %.0f%%, modifier=%.2f\n",
           percent * 100.0f, m_skillCooldownModifier);
}

// ============================================================
// 技能状态更新
// ============================================================
void Player::UpdateSkillCooldown(float deltaTime) {
    if (m_skillCooldownRemaining > 0.0f) {
        m_skillCooldownRemaining -= deltaTime;
        if (m_skillCooldownRemaining <= 0.0f) {
            m_skillCooldownRemaining = 0.0f;
        }
    }
}

void Player::UpdateFullFire(float deltaTime) {
    m_fullFireTimer -= deltaTime;
    if (m_fullFireTimer <= 0.0f) {
        m_fullFireActive = false;
        m_fullFireTimer = 0.0f;
        if (m_currentWeapon) {
            m_currentWeapon->SetFireRateMultiplier(1.0f);  // 恢复射速
        }
        printf("[Player] Full Fire expired.\n");
    }
}

void Player::UpdateRoll(float deltaTime) {
    m_rollTimer += deltaTime;
    float rollDuration = 0.22f;  // 翻滚动画持续 0.22 秒（更快）

    if (m_rollTimer < rollDuration) {
        // 突进移动
        float speed = m_rollDistance / rollDuration;
        Vector2 offset = m_rollDirection * speed * deltaTime;
        m_position += offset;
        // 全程无敌，由 SetInvincibleDuration 管理计时器
    } else {
        // 翻滚结束
        m_isRolling = false;
        m_rollInvincible = false;
        m_rollTimer = 0.0f;
        SetInvincible(false);
        printf("[Player] Roll complete.\n");
    }
}

// ============================================================
// 受到伤害 / 死亡 / 复活
// ============================================================
void Player::ApplyBurning(float duration) {
    BuffManager& buffs = GameManager::GetInstance().GetBuffManager();
    if (buffs.IsFireImmune()) return;  // 火盾免疫燃烧
    Character::ApplyBurning(duration);
}

void Player::ApplyFrozen(float duration) {
    BuffManager& buffs = GameManager::GetInstance().GetBuffManager();
    if (buffs.IsIceImmune()) return;  // 冰盾免疫减速
    Character::ApplyFrozen(duration);
}

void Player::TakeDamage(int damage) {
    // 翻滚期间无敌
    if (m_isRolling && m_rollInvincible) return;

    // 受伤无敌帧
    if (m_damageInvincibleTimer > 0.0f) return;

    // 火盾天赋：+1 护甲
    BuffManager& buffs = GameManager::GetInstance().GetBuffManager();
    int oldArmor = m_armor;
    if (buffs.IsFireImmune()) {
        m_armor += 1;
    }

    Character::TakeDamage(damage);

    // 恢复护甲值
    m_armor = oldArmor;

    // 启动短暂无敌帧
    if (!m_isDead) {
        m_damageInvincibleTimer = DAMAGE_INVINCIBLE_DURATION;
    }
}

void Player::Die() {
    m_isDead = true;
    m_active = false;
    printf("[Player] Died! HP=0\n");
}

void Player::OnRevive() {
    m_isDead = false;
    m_active = true;
    m_hasReviveCharm = false;
    SetHP(GetMaxHP());
    SetMP(GetMaxMP());
    m_damageInvincibleTimer = 0.0f;
    // 清除负面状态
    m_isStunned = false;
    m_isBurning = false;
    m_isFrozen = false;
    m_isInvincible = false;
    m_stunTimer = 0.0f;
    m_burningTimer = 0.0f;
    m_frozenTimer = 0.0f;
    printf("[Player] Revived by charm! HP fully restored.\n");
}
