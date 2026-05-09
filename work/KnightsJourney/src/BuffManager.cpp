// ============================================================
// BuffManager.cpp - 天赋系统管理器
// ============================================================

#include "../include/System/BuffManager.h"
#include "../include/Entity/Player.h"
#include "../include/Core/GameManager.h"
#include <cstdio>
#include <cstring>

BuffManager::BuffManager()
    : m_hasFireShield(false), m_hasIceShield(false), m_hasBloodthirst(false)
    , m_hasCDReduction(false), m_hasSpreadIncrease(false), m_hasDamageBoost(false)
    , m_hasGoldBonus(false), m_hasReviveCharm(false) {}

void BuffManager::Init() {
    // 初始化所有 10 个天赋定义
    BuffData b;

    b.type = BuffType::HP_BOOST; b.name = "生命强化"; b.description = "最大生命值 +4"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::MP_BOOST; b.name = "能量强化"; b.description = "最大能量 +50"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::FIRE_SHIELD; b.name = "火盾"; b.description = "免疫火焰伤害和地面灼烧"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::ICE_SHIELD; b.name = "冰盾"; b.description = "冰系伤害减半，免疫减速"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::BLOODTHIRST; b.name = "嗜血"; b.description = "击杀敌人回复 1 HP"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::CD_REDUCTION; b.name = "CD缩减"; b.description = "技能冷却时间 -40%"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::SPREAD_INCREASE; b.name = "散弹增加"; b.description = "霰弹枪子弹数 +2"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::DAMAGE_BOOST; b.name = "伤害强化"; b.description = "所有武器伤害 +2"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::GOLD_BONUS; b.name = "金币加成"; b.description = "金币掉落翻倍，商店 8 折"; b.isConsumable = false;
    m_buffDefinitions.push_back(b);
    b.type = BuffType::REVIVE_CHARM; b.name = "复活十字章"; b.description = "死亡时自动复活一次"; b.isConsumable = true;
    m_buffDefinitions.push_back(b);

    printf("[BuffManager] Initialized %zu buff definitions.\n", m_buffDefinitions.size());
}

std::vector<BuffData*> BuffManager::RollRandomBuffs(int count) {
    std::vector<BuffData*> result;
    std::vector<BuffData*> available;

    int currentLevel = GameManager::GetInstance().GetCurrentLevel();
    for (auto& b : m_buffDefinitions) {
        if (!HasBuff(b.type) && !b.isConsumed) {
            // 冰盾只在前两关出现
            if (b.type == BuffType::ICE_SHIELD && currentLevel > 2) continue;
            available.push_back(&b);
        }
    }

    int n = (int)available.size();
    if (n == 0) return result;

    int actual = (count < n) ? count : n;
    for (int i = 0; i < actual; ++i) {
        int r = RandomInt(0, (int)available.size() - 1);
        result.push_back(available[r]);
        available.erase(available.begin() + r);
    }
    return result;
}

void BuffManager::ApplyBuff(BuffType type, Player* player) {
    if (HasBuff(type)) return;

    BuffData b;
    b.type = type;
    b.isActive = true;

    switch (type) {
        case BuffType::HP_BOOST:
            if (player) {
                player->SetMaxHP(player->GetMaxHP() + 4);
                player->SetHP(player->GetMaxHP());  // 回满血
            }
            b.name = "生命强化";
            break;
        case BuffType::MP_BOOST:
            if (player) {
                player->SetMaxMP(player->GetMaxMP() + 50);
                player->SetMP(player->GetMaxMP());  // 回满蓝
            }
            b.name = "能量强化";
            break;
        case BuffType::FIRE_SHIELD:  m_hasFireShield = true;  b.name = "火盾"; break;
        case BuffType::ICE_SHIELD:   m_hasIceShield = true;   b.name = "冰盾"; break;
        case BuffType::BLOODTHIRST:  m_hasBloodthirst = true; b.name = "嗜血"; break;
        case BuffType::CD_REDUCTION: m_hasCDReduction = true; b.name = "CD缩减";
            if (player) player->ReduceSkillCooldown(0.40f);
            break;
        case BuffType::SPREAD_INCREASE: m_hasSpreadIncrease = true; b.name = "散弹增加";
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w && w->GetType() == WeaponType::SHOTGUN) {
                        w->SetProjectileCount(w->GetProjectileCount() + 2);
                    }
                }
            }
            break;
        case BuffType::DAMAGE_BOOST:    m_hasDamageBoost = true;    b.name = "伤害强化";
            if (player) {
                // 给当前武器 +2 伤害
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetBaseDamage(w->GetBaseDamage() + 2);
                }
            }
            break;
        case BuffType::GOLD_BONUS:      m_hasGoldBonus = true;      b.name = "金币加成"; break;
        case BuffType::REVIVE_CHARM:
            m_hasReviveCharm = true;
            if (player) player->SetReviveCharm(true);
            b.name = "复活十字章";
            break;
    }

    m_activeBuffs.push_back(b);
    printf("[BuffManager] Buff applied: %s\n", GetBuffName(type));
}

void BuffManager::RemoveBuff(BuffType type, Player* player) {
    for (auto it = m_activeBuffs.begin(); it != m_activeBuffs.end(); ++it) {
        if (it->type == type) {
            m_activeBuffs.erase(it);
            break;
        }
    }

    switch (type) {
        case BuffType::FIRE_SHIELD:  m_hasFireShield = false; break;
        case BuffType::ICE_SHIELD:   m_hasIceShield = false; break;
        case BuffType::BLOODTHIRST:  m_hasBloodthirst = false; break;
        case BuffType::CD_REDUCTION: m_hasCDReduction = false; break;
        case BuffType::SPREAD_INCREASE: m_hasSpreadIncrease = false; break;
        case BuffType::DAMAGE_BOOST:    m_hasDamageBoost = false;    break;
        case BuffType::GOLD_BONUS:      m_hasGoldBonus = false; break;
        case BuffType::REVIVE_CHARM:    m_hasReviveCharm = false; break;
        default: break;
    }
}

bool BuffManager::HasBuff(BuffType type) const {
    for (auto& b : m_activeBuffs) {
        if (b.type == type) return true;
    }
    return false;
}

void BuffManager::ConsumeBuff(BuffType type, Player* player) {
    for (auto& b : m_activeBuffs) {
        if (b.type == type && !b.isConsumed) {
            b.isConsumed = true;
            if (type == BuffType::REVIVE_CHARM) {
                m_hasReviveCharm = false;
                if (player) player->SetReviveCharm(false);
            }
            printf("[BuffManager] Buff consumed: %s\n", GetBuffName(type));
            return;
        }
    }
}

const char* BuffManager::GetBuffName(BuffType type) {
    switch (type) {
        case BuffType::HP_BOOST:       return "生命强化";
        case BuffType::MP_BOOST:       return "能量强化";
        case BuffType::FIRE_SHIELD:    return "火盾";
        case BuffType::ICE_SHIELD:     return "冰盾";
        case BuffType::BLOODTHIRST:    return "嗜血";
        case BuffType::CD_REDUCTION:   return "CD缩减";
        case BuffType::SPREAD_INCREASE: return "散弹增加";
        case BuffType::DAMAGE_BOOST:   return "伤害强化";
        case BuffType::GOLD_BONUS:     return "金币加成";
        case BuffType::REVIVE_CHARM:   return "复活十字章";
        default: return "未知";
    }
}

const char* BuffManager::GetBuffDescription(BuffType type) {
    switch (type) {
        case BuffType::HP_BOOST:       return "最大生命值 +4";
        case BuffType::MP_BOOST:       return "最大能量 +50";
        case BuffType::FIRE_SHIELD:    return "免疫火焰，爆炸+50%";
        case BuffType::ICE_SHIELD:     return "冰伤减半，免疫减速";
        case BuffType::BLOODTHIRST:    return "击杀回复 1 HP";
        case BuffType::CD_REDUCTION:   return "技能CD -40%";
        case BuffType::SPREAD_INCREASE: return "霰弹枪子弹+2";
        case BuffType::DAMAGE_BOOST:   return "所有武器伤害+2";
        case BuffType::GOLD_BONUS:     return "金币翻倍，商店8折";
        case BuffType::REVIVE_CHARM:   return "自动复活一次";
        default: return "";
    }
}

bool BuffManager::IsBuffConsumable(BuffType type) {
    return type == BuffType::REVIVE_CHARM;
}

void BuffManager::Reset() {
    m_activeBuffs.clear();
    m_hasFireShield = false;
    m_hasIceShield = false;
    m_hasBloodthirst = false;
    m_hasCDReduction = false;
    m_hasSpreadIncrease = false;
    m_hasDamageBoost = false;
    m_hasGoldBonus = false;
    m_hasReviveCharm = false;
    printf("[BuffManager] All buffs reset.\n");
}
