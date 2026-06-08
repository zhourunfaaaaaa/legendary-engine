// ============================================================
// Obstacle.cpp - 障碍物实现 (Tree / IcePatch / LavaPool)
// ============================================================

#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Player.h"
#include "../include/Graphics/VisualEffects.h"
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
    VisualFX::DrawPixelShadow(cx, cy + 22, 30, 10);
    // 树干
    VisualFX::DrawPixelRect(cx - 10, cy - 4, cx + 10, cy + 26,
                            RGB(104, 70, 37), RGB(42, 28, 16), 4);
    // 树冠
    VisualFX::DrawPixelRect(cx - 30, cy - 36, cx + 30, cy + 4,
                            RGB(43, 133, 65), RGB(16, 42, 27), 5);
    VisualFX::DrawPixelRect(cx - 20, cy - 52, cx + 20, cy - 20,
                            RGB(55, 154, 72), RGB(16, 42, 27), 5);
    setfillcolor(RGB(93, 190, 84));
    solidrectangle(cx - 12, cy - 45, cx + 4, cy - 37);
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
    VisualFX::DrawPixelShadow(cx, cy + 18, 32, 8);
    VisualFX::DrawPixelRect(cx - 32, cy - 22, cx + 32, cy + 22,
                            RGB(103, 184, 219), RGB(27, 58, 76), 4);
    setfillcolor(RGB(174, 233, 248));
    solidrectangle(cx - 22, cy - 14, cx + 8, cy - 6);
    solidrectangle(cx + 6, cy + 5, cx + 22, cy + 12);
    setlinecolor(RGB(225, 255, 255));
    line(cx - 19, cy - 2, cx - 2, cy + 8);
    line(cx - 2, cy + 8, cx + 18, cy - 9);
    line(cx + 4, cy + 3, cx + 11, cy + 15);
}

void IcePatch::ApplyGroundEffect(Player* player, float deltaTime) {
    if (player) player->ApplyFrozen(0.1f);
}

// ---------- IceBlock ----------
IceBlock::IceBlock() {
    m_obstacleType = ObstacleType::ICE_BLOCK;
    m_blocksBullets = true;
    m_blocksMovement = true;
    SetCollisionSize(27.0f, 27.0f);
}

void IceBlock::Render() {
    int cx = (int)m_position.x;
    int cy = (int)m_position.y;
    VisualFX::DrawPixelShadow(cx, cy + 24, 28, 9);
    VisualFX::DrawPixelRect(cx - 25, cy - 28, cx + 25, cy + 30,
                            RGB(88, 161, 210), RGB(17, 47, 74), 5);
    VisualFX::DrawPixelRect(cx - 17, cy - 46, cx + 17, cy - 18,
                            RGB(132, 218, 244), RGB(17, 47, 74), 4);
    VisualFX::DrawPixelDiamond(cx, cy - 50, 13, RGB(205, 252, 255), RGB(22, 71, 96));
    setfillcolor(RGB(187, 242, 255));
    solidrectangle(cx - 14, cy - 18, cx - 3, cy + 15);
    setfillcolor(RGB(54, 118, 171));
    solidrectangle(cx + 7, cy - 7, cx + 17, cy + 24);
    setlinecolor(RGB(226, 255, 255));
    line(cx - 12, cy - 33, cx - 2, cy + 18);
    line(cx + 6, cy - 26, cx + 15, cy + 11);
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
    VisualFX::DrawPixelShadow(cx, cy + 18, 30, 8);
    VisualFX::DrawPixelRect(cx - 30, cy - 22, cx + 30, cy + 22,
                            RGB(112, 31, 23), RGB(29, 16, 13), 4);
    setfillcolor(RGB(207, 59, 28));
    solidrectangle(cx - 22, cy - 14, cx + 20, cy + 12);
    setfillcolor(RGB(255, 127, 34));
    solidrectangle(cx - 14, cy - 7, cx + 12, cy + 6);
    setfillcolor(RGB(255, 210, 70));
    solidrectangle(cx + 2, cy - 5, cx + 11, cy + 2);
}

void LavaPool::ApplyGroundEffect(Player* player, float deltaTime) {
    if (player && !player->IsBurning()) player->ApplyBurning(2.0f);
}
