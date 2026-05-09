#pragma once

#include "../Core/Common.h"

// ============================================================
// GameObject - 所有游戏实体的抽象基类
// ============================================================
class GameObject {
public:
    GameObject();
    virtual ~GameObject() = default;

    // 每个派生类必须实现自己的更新和绘制
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    // 获取类型名称（调试用）
    virtual const char* GetTypeName() const { return "GameObject"; }

    // 位置
    const Vector2& GetPosition() const { return m_position; }
    void SetPosition(const Vector2& pos) { m_position = pos; }
    void SetPosition(float x, float y) { m_position.x = x; m_position.y = y; }
    float GetX() const { return m_position.x; }
    float GetY() const { return m_position.y; }
    void SetX(float x) { m_position.x = x; }
    void SetY(float y) { m_position.y = y; }

    // 碰撞体
    const AABB& GetAABB() const { return m_aabb; }
    void SetAABB(const AABB& aabb) { m_aabb = aabb; }
    void SyncAABBToPosition();  // 让 AABB 跟随 position
    void SetCollisionSize(float halfW, float halfH);

    // 生命周期
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    bool IsMarkedForDeletion() const { return m_markedForDelete; }
    void MarkForDeletion() { m_markedForDelete = true; }

    // 渲染层级（数值越大越在上层）
    int  GetRenderLayer() const { return m_renderLayer; }
    void SetRenderLayer(int layer) { m_renderLayer = layer; }

    // 实体 ID
    int  GetID() const { return m_id; }

protected:
    Vector2     m_position;          // 世界坐标
    Vector2     m_velocity;          // 速度向量
    AABB        m_aabb;              // 轴对齐碰撞盒
    bool        m_active;            // 是否活跃
    bool        m_markedForDelete;   // 等待删除标记
    int         m_renderLayer;       // 渲染层级
    int         m_id;                // 唯一 ID

    static int  s_nextID;            // 全局自增 ID 生成器
};
