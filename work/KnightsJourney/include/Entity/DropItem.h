#pragma once

#include "GameObject.h"

// ============================================================
// DropItem - 掉落物（金币、血瓶、蓝瓶、武器）
// ============================================================
class DropItem : public GameObject {
public:
    DropItem();
    virtual ~DropItem() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "DropItem"; }

    DropType GetDropType() const { return m_dropType; }
    void SetDropType(DropType type) { m_dropType = type; }

    // 数值（金币数量 / 回复量 / 武器类型）
    int  GetValue() const { return m_value; }
    void SetValue(int val) { m_value = val; }

    // 武器掉落
    WeaponType GetWeaponType() const { return m_weaponType; }
    void SetWeaponType(WeaponType wt) { m_weaponType = wt; }

    // 拾取效果
    void OnPickup(Player* player);

    // 吸引效果（自动飞向玩家）
    bool IsMagnetized() const { return m_magnetized; }
    void SetMagnetized(bool v) { m_magnetized = v; }
    float GetMagnetSpeed() const { return m_magnetSpeed; }

    // 存活时间（超时消失）
    float GetAliveTime() const { return m_aliveTime; }
    void SetMaxAliveTime(float t) { m_maxAliveTime = t; }
    bool IsExpired() const { return m_aliveTime >= m_maxAliveTime; }

    // 弹跳动画参数
    float GetBounceOffset() const { return m_bounceOffset; }

private:
    DropType    m_dropType;
    int         m_value;
    WeaponType  m_weaponType;

    bool        m_magnetized;
    float       m_magnetSpeed;
    float       m_magnetRange;

    float       m_aliveTime;
    float       m_maxAliveTime;

    // 动画
    float       m_bounceTimer;
    float       m_bounceOffset;
};
