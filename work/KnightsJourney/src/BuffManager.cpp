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
    , m_hasGoldBonus(false), m_hasReviveCharm(false)
    , m_hasPotionMastery(false), m_hasBossHunter(false), m_hasTreasureInstinct(false) {}

void BuffManager::Init() {
    m_buffDefinitions.clear();
    BuffData b;

    auto add = [&](BuffType type, const char* name, const char* desc, bool consumable = false) {
        b = BuffData();
        b.type = type;
        b.name = name;
        b.description = desc;
        b.isConsumable = consumable;
        m_buffDefinitions.push_back(b);
    };

    add(BuffType::HP_BOOST, "树心护符", "最大生命 +3，并立即回满");
    add(BuffType::MP_BOOST, "蓝晶电池", "最大能量 +60，并立即回满");
    add(BuffType::FIRE_SHIELD, "熔火靴", "免疫火焰地面，爆炸范围提升");
    add(BuffType::ICE_SHIELD, "霜纹披肩", "免疫冰冻减速，冰伤减半");
    add(BuffType::BLOODTHIRST, "红月契约", "击杀敌人回复 1 HP");
    add(BuffType::CD_REDUCTION, "快手护腕", "技能冷却 -30%");
    add(BuffType::SPREAD_INCREASE, "扩容弹巢", "多弹武器额外 +1 发");
    add(BuffType::DAMAGE_BOOST, "磨刃核心", "所有当前武器伤害 +1");
    add(BuffType::GOLD_BONUS, "商会印章", "金币 +60%，商店 85 折");
    add(BuffType::REVIVE_CHARM, "余烬吊坠", "死亡时自动复活一次", true);
    add(BuffType::SHIELD_BOOST, "符文圆盾", "最大护盾 +2，并立即补满");
    add(BuffType::MOVE_SPEED_BOOST, "风行鞋", "移动速度 +8%");
    add(BuffType::ARMOR_PLATING, "黄铜甲片", "护甲 +1，适合近战压血线");
    add(BuffType::CRIT_BOOST, "幸运撞针", "当前武器暴击率 +10%");
    add(BuffType::FIRE_RATE_BOOST, "齿轮扳机", "当前武器射速 +12%");
    add(BuffType::ENERGY_SAVER, "节能线圈", "当前武器蓝耗 -1，最低为 0");
    add(BuffType::EXTRA_PROJECTILE, "分裂棱镜", "当前武器弹丸数 +1");
    add(BuffType::POTION_MASTERY, "药剂腰带", "药水恢复量提升");
    add(BuffType::BOSS_HUNTER, "猎首刻印", "当前武器伤害 +2，但只在Boss后出现");
    add(BuffType::TREASURE_INSTINCT, "寻宝罗盘", "后续宝箱更容易出高阶武器");

    printf("[BuffManager] Initialized %zu buff definitions.\n", m_buffDefinitions.size());
}

std::vector<BuffData*> BuffManager::RollRandomBuffs(int count) {
    std::vector<BuffData*> result;
    std::vector<BuffData*> available;

    int currentLevel = GameManager::GetInstance().GetCurrentLevel();
    for (auto& b : m_buffDefinitions) {
        if (!HasBuff(b.type) && !b.isConsumed) {
            if (b.type == BuffType::ICE_SHIELD && currentLevel >= 6) continue;
            if (b.type == BuffType::BOSS_HUNTER && currentLevel < 3) continue;
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
                player->SetMaxHP(player->GetMaxHP() + 3);
                player->SetHP(player->GetMaxHP());  // 回满血
            }
            break;
        case BuffType::MP_BOOST:
            if (player) {
                player->SetMaxMP(player->GetMaxMP() + 60);
                player->SetMP(player->GetMaxMP());  // 回满蓝
            }
            break;
        case BuffType::FIRE_SHIELD:  m_hasFireShield = true;  break;
        case BuffType::ICE_SHIELD:   m_hasIceShield = true;   break;
        case BuffType::BLOODTHIRST:  m_hasBloodthirst = true; break;
        case BuffType::CD_REDUCTION: m_hasCDReduction = true;
            if (player) player->ReduceSkillCooldown(0.30f);
            break;
        case BuffType::SPREAD_INCREASE: m_hasSpreadIncrease = true;
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w && w->GetProjectileCount() > 1) {
                        w->SetProjectileCount(w->GetProjectileCount() + 1);
                    }
                }
            }
            break;
        case BuffType::DAMAGE_BOOST:    m_hasDamageBoost = true;
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetBaseDamage(w->GetBaseDamage() + 1);
                }
            }
            break;
        case BuffType::GOLD_BONUS:      m_hasGoldBonus = true; break;
        case BuffType::REVIVE_CHARM:
            m_hasReviveCharm = true;
            if (player) player->SetReviveCharm(true);
            break;
        case BuffType::SHIELD_BOOST:
            if (player) {
                player->SetMaxShield(player->GetMaxShield() + 2);
                player->SetShield(player->GetMaxShield());
            }
            break;
        case BuffType::MOVE_SPEED_BOOST:
            if (player) player->SetMoveSpeed(player->GetMoveSpeed() * 1.08f);
            break;
        case BuffType::ARMOR_PLATING:
            if (player) player->SetArmor(player->GetArmor() + 1);
            break;
        case BuffType::CRIT_BOOST:
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetCritChance(w->GetCritChance() + 0.10f);
                }
            }
            break;
        case BuffType::FIRE_RATE_BOOST:
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetFireRate(w->GetFireRate() * 1.12f);
                }
            }
            break;
        case BuffType::ENERGY_SAVER:
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetMPCost(w->GetMPCost() - 1);
                }
            }
            break;
        case BuffType::EXTRA_PROJECTILE:
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetProjectileCount(w->GetProjectileCount() + 1);
                }
            }
            break;
        case BuffType::POTION_MASTERY:
            m_hasPotionMastery = true;
            break;
        case BuffType::BOSS_HUNTER:
            m_hasBossHunter = true;
            if (player) {
                for (int i = 0; i < player->GetWeaponCount(); ++i) {
                    Weapon* w = player->GetWeapon(i);
                    if (w) w->SetBaseDamage(w->GetBaseDamage() + 2);
                }
            }
            break;
        case BuffType::TREASURE_INSTINCT:
            m_hasTreasureInstinct = true;
            break;
    }

    b.name = GetBuffName(type);
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
        case BuffType::POTION_MASTERY:  m_hasPotionMastery = false; break;
        case BuffType::BOSS_HUNTER:      m_hasBossHunter = false; break;
        case BuffType::TREASURE_INSTINCT:m_hasTreasureInstinct = false; break;
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
        case BuffType::HP_BOOST:       return "树心护符";
        case BuffType::MP_BOOST:       return "蓝晶电池";
        case BuffType::FIRE_SHIELD:    return "熔火靴";
        case BuffType::ICE_SHIELD:     return "霜纹披肩";
        case BuffType::BLOODTHIRST:    return "红月契约";
        case BuffType::CD_REDUCTION:   return "快手护腕";
        case BuffType::SPREAD_INCREASE: return "扩容弹巢";
        case BuffType::DAMAGE_BOOST:   return "磨刃核心";
        case BuffType::GOLD_BONUS:     return "商会印章";
        case BuffType::REVIVE_CHARM:   return "余烬吊坠";
        case BuffType::SHIELD_BOOST:   return "符文圆盾";
        case BuffType::MOVE_SPEED_BOOST:return "风行鞋";
        case BuffType::ARMOR_PLATING:  return "黄铜甲片";
        case BuffType::CRIT_BOOST:     return "幸运撞针";
        case BuffType::FIRE_RATE_BOOST:return "齿轮扳机";
        case BuffType::ENERGY_SAVER:   return "节能线圈";
        case BuffType::EXTRA_PROJECTILE:return "分裂棱镜";
        case BuffType::POTION_MASTERY: return "药剂腰带";
        case BuffType::BOSS_HUNTER:    return "猎首刻印";
        case BuffType::TREASURE_INSTINCT:return "寻宝罗盘";
        default: return "未知";
    }
}

const char* BuffManager::GetBuffDescription(BuffType type) {
    switch (type) {
        case BuffType::HP_BOOST:       return "最大生命 +3，并立即回满";
        case BuffType::MP_BOOST:       return "最大能量 +60，并立即回满";
        case BuffType::FIRE_SHIELD:    return "免疫火焰地面，爆炸范围提升";
        case BuffType::ICE_SHIELD:     return "冰伤减半，免疫减速";
        case BuffType::BLOODTHIRST:    return "击杀回复 1 HP";
        case BuffType::CD_REDUCTION:   return "技能冷却 -30%";
        case BuffType::SPREAD_INCREASE: return "多弹武器额外 +1 发";
        case BuffType::DAMAGE_BOOST:   return "当前武器伤害 +1";
        case BuffType::GOLD_BONUS:     return "金币 +60%，商店85折";
        case BuffType::REVIVE_CHARM:   return "自动复活一次";
        case BuffType::SHIELD_BOOST:   return "最大护盾 +2，并立即补满";
        case BuffType::MOVE_SPEED_BOOST:return "移动速度 +8%";
        case BuffType::ARMOR_PLATING:  return "护甲 +1";
        case BuffType::CRIT_BOOST:     return "当前武器暴击率 +10%";
        case BuffType::FIRE_RATE_BOOST:return "当前武器射速 +12%";
        case BuffType::ENERGY_SAVER:   return "当前武器蓝耗 -1";
        case BuffType::EXTRA_PROJECTILE:return "当前武器弹丸数 +1";
        case BuffType::POTION_MASTERY: return "药水恢复量提升";
        case BuffType::BOSS_HUNTER:    return "当前武器伤害 +2";
        case BuffType::TREASURE_INSTINCT:return "宝箱更容易出高阶武器";
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
    m_hasPotionMastery = false;
    m_hasBossHunter = false;
    m_hasTreasureInstinct = false;
    printf("[BuffManager] All buffs reset.\n");
}
