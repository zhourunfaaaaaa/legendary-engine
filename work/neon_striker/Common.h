#pragma once

#include <windows.h>
#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <ctime>

// ============================================================
// Common.h - 全局枚举、常量、基础类型定义
// Neon Striker 赛博朋克弹幕射击
// ============================================================

// ---------- 窗口与渲染常量 ----------
constexpr int   WINDOW_WIDTH        = 1600;
constexpr int   WINDOW_HEIGHT       = 900;
constexpr float PI                  = 3.14159265358979323846f;

// ---------- 游戏数值常量 ----------
constexpr int   STAGE2_KILLS        = 20;
constexpr int   STAGE3_KILLS        = 50;
constexpr float LASER_DMG_TICK      = 12.0f;
constexpr float LASER_TICK_GAP      = 0.05f;
constexpr float LASER_WIDTH         = 90.0f;
constexpr float LASER_REACH         = 1200.0f;
constexpr float LIFESTEAL_PCT       = 0.03f;

constexpr int   BOSS_HP             = 14400;
constexpr float BOSS_STAGE2_TIME    = 20.0f;
constexpr float BOSS_STAGE3_TIME    = 40.0f;
constexpr float BOSS_LASER_TIME     = 30.0f;
constexpr float BOSS_SCATTER_TIME   = 15.0f;
constexpr float BOSS_SHOOT_GAP      = 1.5f;
constexpr float BOSS_SPEED          = 40.0f;

constexpr float PLAYER_SPEED        = 1800.0f;
constexpr int   PLAYER_HP           = 100;
constexpr int   PLAYER_MAX_ENERGY   = 100;
constexpr float PLAYER_SIZE         = 18.0f;
constexpr float PLAYER_SHOOT_INTERVAL = 0.08f;

constexpr float ULTIMATE_DURATION   = 2.0f;
constexpr float SLOW_MOTION_FACTOR  = 0.3f;
constexpr float SLOW_MOTION_RECOVER = 3.0f;

constexpr float SPAWN_RATE_BASE     = 1.5f;
constexpr float SPAWN_RATE_WAVE     = 0.2f;
constexpr int   MAX_CONCURRENT_BASE = 5;
constexpr int   MAX_CONCURRENT_WAVE = 2;

constexpr float COMBO_TIMEOUT       = 1.5f;
constexpr float INVINCIBLE_TIME     = 0.5f;
constexpr float INPUT_LOCK_TIME     = 0.25f;
constexpr float MAX_DELTA_TIME      = 0.05f;
constexpr float MAX_FRAME_TIME      = 0.1f;
constexpr float TARGET_FRAME_TIME   = 1.0f / 60.0f;

constexpr int   NEXT_INFINITE_BOSS  = 80;
constexpr int   INFINITE_BOSS_HP    = 8000;

// ---------- 游戏状态机 ----------
enum class GameState {
    MAIN_MENU,
    INTRO,
    PLAYING,
    PAUSED,
    GAME_OVER,
    VICTORY,
    DEFEAT
};
// PAUSED 仅用于 ESC 暂停，游戏逻辑冻结但仍在渲染

// ---------- 武器类型 ----------
enum class WeaponType {
    BEAM,
    PULSE,
    SCATTER
};

// ---------- 敌人类型 ----------
enum class EnemyType {
    SMALL,
    NORMAL,
    LARGE,
    FAST,
    SHIELD,
    SPLITTER,
    SNIPER
};

// ---------- 子弹归属 ----------
enum class BulletFaction {
    PLAYER,
    ENEMY
};

// ---------- 子弹特殊行为 ----------
enum class BulletBehavior {
    NONE,
    BOUNCE,
    LASER_ORB
};

// ---------- 敌人射击模式 ----------
enum class ShootPattern {
    SINGLE,
    SPREAD,
    SPIRAL,
    LASER,
    SNIPER
};

// ---------- BOSS阶段 ----------
enum class BossPhase {
    PHASE_1,
    PHASE_2,
    PHASE_3
};

// ---------- BOSS攻击类型 ----------
enum class BossAttack {
    RING_BURST,
    AIMED_FAN,
    WARN_BEAM,
    BOUNCE_SHOT,
    LASER_ORB
};

// ---------- 基础结构体 ----------
struct Vector2 {
    float x, y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }

    Vector2 Normalized() const {
        float len = Length();
        if (len > 0.0001f) return Vector2(x / len, y / len);
        return Vector2(0.0f, 0.0f);
    }

    float Dot(const Vector2& other) const { return x * other.x + y * other.y; }

    static float Distance(const Vector2& a, const Vector2& b) { return (a - b).Length(); }
    static float DistanceSquared(const Vector2& a, const Vector2& b) { return (a - b).LengthSquared(); }
};

// AABB 碰撞盒
struct AABB {
    float x, y;
    float halfW, halfH;

    AABB() : x(0), y(0), halfW(16), halfH(16) {}
    AABB(float cx, float cy, float hw, float hh)
        : x(cx), y(cy), halfW(hw), halfH(hh) {}

    float Left()   const { return x - halfW; }
    float Right()  const { return x + halfW; }
    float Top()    const { return y - halfH; }
    float Bottom() const { return y + halfH; }

    bool Intersects(const AABB& other) const {
        return !(Right() < other.Left()  || Left()  > other.Right() ||
                 Bottom() < other.Top() || Top()    > other.Bottom());
    }
};

// 圆形碰撞盒
struct CircleCollider {
    float x, y;
    float radius;

    CircleCollider() : x(0), y(0), radius(8) {}
    CircleCollider(float cx, float cy, float r) : x(cx), y(cy), radius(r) {}

    bool Intersects(const CircleCollider& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float distSq = dx * dx + dy * dy;
        float radii = radius + other.radius;
        return distSq <= radii * radii;
    }

    bool IntersectsAABB(const AABB& aabb) const {
        float closestX = (std::max)(aabb.Left(), (std::min)(x, aabb.Right()));
        float closestY = (std::max)(aabb.Top(),  (std::min)(y, aabb.Bottom()));
        float dx = x - closestX;
        float dy = y - closestY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
};

// ---------- 工具函数 ----------
inline float DegToRad(float deg) { return deg * (PI / 180.0f); }

inline float RadToDeg(float rad) { return rad * (180.0f / PI); }

inline int RandomInt(int min, int max) {
    static std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

inline float RandomFloat(float min, float max) {
    static std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)) + 1);
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

template<typename T>
T Clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// 限制值到范围内 (float版本)
inline float ClampFloat(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// 前向声明
class Player;
class Enemy;
class Boss;
class PlayerBullet;
class EnemyBullet;
class Particle;
