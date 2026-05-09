// ============================================================
// Character.cpp - 角色基类实现（HP/MP/状态效果/伤害结算）
// ============================================================

#include "../include/Entity/Character.h"
#include <algorithm>
#include <cstdio>

Character::Character()
    : m_hp(10)
    , m_maxHP(10)
    , m_mp(100)
    , m_maxMP(100)
    , m_moveSpeed(200.0f)
    , m_armor(0)
    , m_shield(0)
    , m_maxShield(0)
    , m_shieldRegenTimer(0.0f)
    , m_isDead(false)
    , m_moveDirection(0.0f, 0.0f)
    , m_isStunned(false)
    , m_isBurning(false)
    , m_isFrozen(false)
    , m_isInvincible(false)
    , m_stunTimer(0.0f)
    , m_burningTimer(0.0f)
    , m_burningTickTimer(0.0f)
    , m_frozenTimer(0.0f)
    , m_invincibleTimer(0.0f)
    , m_origMoveSpeed(200.0f) {
    SetRenderLayer(5);  // 角色渲染在中间层
}

// ============================================================
// HP / MP 读写（含边界钳制）
// ============================================================
void Character::SetHP(int hp) {
    m_hp = hp;
    if (m_hp < 0)  m_hp = 0;
    if (m_hp > m_maxHP) m_hp = m_maxHP;
}

void Character::SetMaxHP(int maxHP) {
    m_maxHP = maxHP;
    if (m_maxHP < 1) m_maxHP = 1;
    if (m_hp > m_maxHP) m_hp = m_maxHP;
}

void Character::SetMP(int mp) {
    m_mp = mp;
    if (m_mp < 0)  m_mp = 0;
    if (m_mp > m_maxMP) m_mp = m_maxMP;
}

void Character::SetMaxMP(int maxMP) {
    m_maxMP = maxMP;
    if (m_maxMP < 0) m_maxMP = 0;
    if (m_mp > m_maxMP) m_mp = m_maxMP;
}

// ============================================================
// 护盾
// ============================================================
void Character::SetShield(int shield) {
    m_shield = shield;
    if (m_shield < 0) m_shield = 0;
    if (m_shield > m_maxShield) m_shield = m_maxShield;
}

void Character::SetMaxShield(int maxShield) {
    m_maxShield = maxShield;
    if (m_maxShield < 0) m_maxShield = 0;
    if (m_shield > m_maxShield) m_shield = m_maxShield;
}

void Character::RestoreShield(int amount) {
    m_shield += amount;
    if (m_shield > m_maxShield) m_shield = m_maxShield;
}

// ============================================================
// 伤害结算（护盾 → 护甲减伤 → HP）
// ============================================================
void Character::TakeDamage(int damage) {
    if (m_isDead) return;
    if (m_isInvincible) return;

    // 重置护盾恢复计时器
    m_shieldRegenTimer = 0.0f;

    int remaining = damage;

    // 1. 先扣护盾
    if (m_shield > 0) {
        if (remaining <= m_shield) {
            m_shield -= remaining;
            return;  // 护盾完全吸收
        }
        remaining -= m_shield;
        m_shield = 0;
    }

    // 2. 护甲减伤（每点护甲减免 1 点伤害，最低保留 1 点伤害）
    int actualDamage = remaining - m_armor;
    if (actualDamage < 0) actualDamage = 0;

    // 3. 扣 HP
    m_hp -= actualDamage;
    if (m_hp <= 0) {
        m_hp = 0;
        Die();
    }
}

void Character::Heal(int amount) {
    if (m_isDead) return;
    if (amount <= 0) return;

    m_hp += amount;
    if (m_hp > m_maxHP) m_hp = m_maxHP;
}

void Character::Die() {
    m_isDead = true;
    m_active = false;
    MarkForDeletion();
}

// ============================================================
// 状态效果施加
// ============================================================
void Character::ApplyBurning(float duration) {
    m_isBurning = true;
    m_burningTimer = duration;
    m_burningTickTimer = 0.5f;  // 0.5秒后才第一次扣血，避免立即伤害
}

void Character::ApplyFrozen(float duration) {
    if (!m_isFrozen) {
        m_origMoveSpeed = m_moveSpeed;
        m_moveSpeed *= 0.5f;
    }
    m_isFrozen = true;
    m_frozenTimer = duration;
}

void Character::ApplyStun(float duration) {
    m_isStunned = true;
    m_stunTimer = duration;
}

void Character::SetInvincibleDuration(float duration) {
    m_isInvincible = true;
    m_invincibleTimer = duration;
}

// ============================================================
// 状态效果更新（每帧调用）
// ============================================================
void Character::UpdateStatusEffects(float deltaTime) {
    // ---- 晕眩 ----
    if (m_isStunned) {
        m_stunTimer -= deltaTime;
        if (m_stunTimer <= 0.0f) {
            m_isStunned = false;
            m_stunTimer = 0.0f;
        }
    }

    // ---- 燃烧 ----
    if (m_isBurning) {
        m_burningTimer -= deltaTime;
        m_burningTickTimer -= deltaTime;

        // 每 0.5 秒扣一次血
        if (m_burningTickTimer <= 0.0f) {
            m_hp -= 1;
            if (m_hp <= 0) {
                m_hp = 0;
                Die();
                return;  // 死了就不继续处理状态了
            }
            m_burningTickTimer = 0.5f;
        }

        if (m_burningTimer <= 0.0f) {
            m_isBurning = false;
            m_burningTimer = 0.0f;
            m_burningTickTimer = 0.0f;
        }
    }

    // ---- 冰冻 ----
    if (m_isFrozen) {
        m_frozenTimer -= deltaTime;
        if (m_frozenTimer <= 0.0f) {
            m_isFrozen = false;
            m_frozenTimer = 0.0f;
            m_moveSpeed = m_origMoveSpeed;  // 恢复原始速度
        }
    }

    // ---- 无敌 ----
    if (m_isInvincible) {
        m_invincibleTimer -= deltaTime;
        if (m_invincibleTimer <= 0.0f) {
            m_isInvincible = false;
            m_invincibleTimer = 0.0f;
        }
    }

    // ---- 护盾恢复（受伤后 2 秒开始，每秒恢复 1 点） ----
    if (m_maxShield > 0 && m_shield < m_maxShield) {
        m_shieldRegenTimer += deltaTime;
        if (m_shieldRegenTimer >= 2.0f) {
            // 每秒钟恢复 1 点
            float regenSinceDelay = m_shieldRegenTimer - 2.0f;
            int regenAmount = (int)regenSinceDelay;  // 每秒 1 点
            if (regenAmount > 0) {
                m_shield += regenAmount;
                if (m_shield > m_maxShield) m_shield = m_maxShield;
                m_shieldRegenTimer = 2.0f;  // 保留延迟部分
            }
        }
    }
}
