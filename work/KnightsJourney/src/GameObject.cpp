// ============================================================
// GameObject.cpp - 实体抽象基类实现
// ============================================================

#include "../include/Entity/GameObject.h"
#include <cstdio>

// 静态 ID 计数器初始化
int GameObject::s_nextID = 0;

GameObject::GameObject()
    : m_position(0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f)
    , m_aabb(0.0f, 0.0f, 16.0f, 16.0f)    // 默认碰撞盒 32x32
    , m_active(true)
    , m_markedForDelete(false)
    , m_renderLayer(0)
    , m_id(s_nextID++) {
}

void GameObject::SyncAABBToPosition() {
    m_aabb.x = m_position.x;
    m_aabb.y = m_position.y;
}

void GameObject::SetCollisionSize(float halfW, float halfH) {
    m_aabb.halfW = halfW;
    m_aabb.halfH = halfH;
    SyncAABBToPosition();
}
