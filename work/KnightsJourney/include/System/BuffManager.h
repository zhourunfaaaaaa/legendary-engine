#pragma once

#include "../Core/Common.h"
#include <vector>
#include <string>

class Player;

// ============================================================
// BuffData - 单个天赋的运行时数据
// ============================================================
struct BuffData {
    BuffType    type;
    std::string name;
    std::string description;
    bool        isActive;
    bool        isConsumable;        // 消耗品（如复活十字章）
    bool        isConsumed;          // 是否已消耗

    BuffData() : type(BuffType::HP_BOOST), isActive(false),
                 isConsumable(false), isConsumed(false) {}
};

// ============================================================
// BuffManager - 天赋 (Buff) 系统管理器
// 负责天赋选择界面、天赋应用/移除、效果叠加
// ============================================================
class BuffManager {
public:
    BuffManager();
    ~BuffManager() = default;

    // 初始化所有天赋数据
    void Init();

    // 随机抽取 N 个天赋（排除已拥有的）
    std::vector<BuffData*> RollRandomBuffs(int count = 3);

    // 应用天赋
    void ApplyBuff(BuffType type, Player* player);

    // 移除天赋
    void RemoveBuff(BuffType type, Player* player);

    // 检查是否拥有某个天赋
    bool HasBuff(BuffType type) const;

    // 获取所有已激活天赋
    const std::vector<BuffData>& GetActiveBuffs() const { return m_activeBuffs; }

    // 获取所有天赋定义
    const std::vector<BuffData>& GetAllBuffDefs() const { return m_buffDefinitions; }

    // 获取天赋名称/描述
    static const char* GetBuffName(BuffType type);
    static const char* GetBuffDescription(BuffType type);
    static bool IsBuffConsumable(BuffType type);

    // 消耗型天赋处理（如复活十字章触发）
    void ConsumeBuff(BuffType type, Player* player);

    // 天赋效果查询
    bool IsFireImmune() const { return m_hasFireShield; }
    bool IsIceImmune() const { return m_hasIceShield; }
    bool HasBloodthirst() const { return m_hasBloodthirst; }
    bool HasCDReduction() const { return m_hasCDReduction; }
    bool HasSpreadIncrease() const { return m_hasSpreadIncrease; }
    bool HasDamageBoost() const { return m_hasDamageBoost; }
    bool HasGoldBonus() const { return m_hasGoldBonus; }
    bool HasReviveCharm() const { return m_hasReviveCharm; }
    bool HasPotionMastery() const { return m_hasPotionMastery; }
    bool HasBossHunter() const { return m_hasBossHunter; }
    bool HasTreasureInstinct() const { return m_hasTreasureInstinct; }

    float GetCDReductionPercent() const { return m_hasCDReduction ? 0.30f : 0.0f; }
    float GetGoldMultiplier() const { return m_hasGoldBonus ? 1.6f : 1.0f; }
    float GetShopDiscount() const { return m_hasGoldBonus ? 0.85f : 1.0f; }
    float GetExplosionRadiusBonus() const { return m_hasFireShield ? 1.5f : 1.0f; }
    float GetIceDamageReduction() const { return m_hasIceShield ? 0.50f : 1.0f; }

    // 重置（新游戏开始时）
    void Reset();

private:
    std::vector<BuffData> m_buffDefinitions;   // 所有天赋定义
    std::vector<BuffData> m_activeBuffs;       // 当前激活的天赋

    // 快速查询标记
    bool m_hasFireShield;
    bool m_hasIceShield;
    bool m_hasBloodthirst;
    bool m_hasCDReduction;
    bool m_hasSpreadIncrease;
    bool m_hasDamageBoost;
    bool m_hasGoldBonus;
    bool m_hasReviveCharm;
    bool m_hasPotionMastery;
    bool m_hasBossHunter;
    bool m_hasTreasureInstinct;
};
