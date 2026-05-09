#pragma once

#include "GameObject.h"

// ============================================================
// Door - 房间之间的门，清理怪物后解锁
// ============================================================
class Door : public GameObject {
public:
    Door();
    virtual ~Door() = default;

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual const char* GetTypeName() const override { return "Door"; }

    // 门状态
    bool IsOpen()   const { return m_isOpen; }
    bool IsLocked() const { return m_isLocked; }

    // 解锁并打开
    void Unlock();
    void Open();
    void Close();

    // 方向
    DoorDirection GetDoorDirection() const { return m_direction; }
    void SetDoorDirection(DoorDirection dir) { m_direction = dir; }

    // 关联房间坐标
    RoomCoord GetTargetRoom() const { return m_targetRoom; }
    void SetTargetRoom(const RoomCoord& coord) { m_targetRoom = coord; }

    // 玩家是否在门前（触发切换房间）
    bool IsPlayerNearby(const Vector2& playerPos) const;

private:
    bool          m_isOpen;
    bool          m_isLocked;
    DoorDirection m_direction;
    RoomCoord     m_targetRoom;

    float         m_openTimer;
    static constexpr float DOOR_INTERACT_RANGE = 48.0f;
};
