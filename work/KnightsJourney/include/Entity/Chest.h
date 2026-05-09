#pragma once

#include "GameObject.h"

// ============================================================
// Chest - 宝箱（奖励房 / 精英房掉落）
// ============================================================
class Chest : public GameObject {
public:
    Chest();
    virtual ~Chest() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Chest"; }

    // 宝箱状态
    bool IsOpened() const { return m_isOpened; }
    bool IsMimic()  const { return m_isMimic; }

    // 设置宝箱怪概率
    void SetMimicChance(float chance) { m_mimicChance = chance; }
    float GetMimicChance() const { return m_mimicChance; }

    // 开启宝箱（返回 true 表示是宝箱怪）
    bool Open();

    // 设置宝箱内容
    void SetAsWeaponDrop(WeaponType wt);
    void SetAsHPPotion(int amount);
    void SetAsMPPotion(int amount);
    void SetAsGold(int amount);
    void SetAsMimic();

    // 读取内容
    DropType GetContentType() const { return m_contentType; }
    WeaponType GetContainedWeapon() const { return m_containedWeapon; }
    int GetContentValue() const { return m_contentValue; }

    // 来源房间类型
    RoomType GetSourceRoom() const { return m_sourceRoom; }
    void SetSourceRoom(RoomType room) { m_sourceRoom = room; }

private:
    bool        m_isOpened;
    bool        m_isMimic;
    float       m_mimicChance;

    DropType    m_contentType;
    WeaponType  m_containedWeapon;
    int         m_contentValue;

    RoomType    m_sourceRoom;

    // 动画
    float       m_openAnimationTimer;
    bool        m_isAnimating;
};
