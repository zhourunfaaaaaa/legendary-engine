// ============================================================
// Obstacle.cpp - 障碍物实现 (Tree / IcePatch / LavaPool)
// ============================================================

#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Player.h"
#include <graphics.h>

// ---------- Obstacle 基类 ----------
Obstacle::Obstacle()
    : m_obstacleType(ObstacleType::TREE), m_blocksBullets(true)
    , m_blocksMovement(true), m_frictionModifier(1.0f) {
    SetRenderLayer(2);
    SetCollisionSize(30.0f, 30.0f);
}

void Obstacle::Update(float deltaTime) { SyncAABBToPosition(); }

void Obstacle::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    setfillcolor(RGB(100, 100, 100));
    solidrectangle(cx - 30, cy - 30, cx + 30, cy + 30);
}

// ---------- Tree ----------
Tree::Tree() {
    m_obstacleType = ObstacleType::TREE;
    m_blocksBullets = true;
    m_blocksMovement = true;
    SetCollisionSize(28.0f, 28.0f);
}

void Tree::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    // 树干
    setfillcolor(RGB(80, 50, 20));
    solidrectangle(cx - 8, cy - 8, cx + 8, cy + 14);
    // 树冠
    setfillcolor(RGB(30, 140, 50));
    solidcircle(cx, cy - 8, 22);
    setfillcolor(RGB(20, 120, 40));
    solidcircle(cx - 8, cy - 4, 14);
    solidcircle(cx + 8, cy - 4, 14);
}

// ---------- IcePatch ----------
IcePatch::IcePatch() {
    m_obstacleType = ObstacleType::ICE_PATCH;
    m_blocksBullets = false;
    m_blocksMovement = false;
    m_frictionModifier = 0.15f;
    SetCollisionSize(32.0f, 32.0f);
}

void IcePatch::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    setfillcolor(RGB(150, 210, 240));
    solidrectangle(cx - 28, cy - 28, cx + 28, cy + 28);
    setfillcolor(RGB(200, 240, 255));
    solidrectangle(cx - 20, cy - 20, cx + 20, cy + 20);
}

void IcePatch::ApplyGroundEffect(Player* player, float deltaTime) {
    if (player) player->ApplyFrozen(0.1f);
}

// ---------- LavaPool ----------
LavaPool::LavaPool()
    : m_lifetime(5.0f), m_age(0.0f) {
    m_obstacleType = ObstacleType::LAVA_POOL;
    m_blocksBullets = false;
    m_blocksMovement = false;
    SetCollisionSize(28.0f, 28.0f);
}

void LavaPool::Update(float deltaTime) {
    Obstacle::Update(deltaTime);
    m_age += deltaTime;
    if (m_age >= m_lifetime) {
        MarkForDeletion();
    }
}

void LavaPool::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    setfillcolor(RGB(180, 40, 10));
    solidrectangle(cx - 24, cy - 24, cx + 24, cy + 24);
    setfillcolor(RGB(255, 100, 20));
    solidrectangle(cx - 16, cy - 16, cx + 16, cy + 16);
    setfillcolor(RGB(255, 200, 60));
    solidrectangle(cx - 8, cy - 8, cx + 8, cy + 8);
}

void LavaPool::ApplyGroundEffect(Player* player, float deltaTime) {
    if (player && !player->IsBurning()) player->ApplyBurning(2.0f);
}
