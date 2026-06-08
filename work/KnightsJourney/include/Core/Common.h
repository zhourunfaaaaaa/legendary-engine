#pragma once

#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <ctime>

// ============================================================
// Common.h - 全局枚举、常量、基础类型定义
// ============================================================

// ---------- 窗口与渲染常量 ----------
constexpr int   WINDOW_WIDTH        = 1600;
constexpr int   WINDOW_HEIGHT       = 900;
constexpr int   TILE_SIZE           = 64;
constexpr int   ROOM_COLS           = 7;
constexpr int   ROOM_ROWS           = 7;
constexpr int   ROOM_WIDTH          = 1200;
constexpr int   ROOM_HEIGHT         = 800;
constexpr float ROOM_WALL_MARGIN    = 54.0f;
constexpr float ROOM_ENTITY_MARGIN  = 68.0f;
constexpr float ROOM_BOSS_MARGIN    = 88.0f;
constexpr float PI                  = 3.14159265358979323846f;

// ---------- 游戏状态机 ----------
enum class GameState {
    MAIN_MENU,
    CHARACTER_SELECT,
    PLAYING,
    PAUSED,
    SHOP,
    REWARD,
    TALENT_SELECT,
    WEAPON_SELECT,       // 武器满时选择替换
    GAME_OVER,
    VICTORY,
    TRANSITION           // 房间切换过渡动画
};

// ---------- 职业枚举 ----------
enum class Profession {
    KNIGHT,
    ROGUE,
    MAGE
};

// ---------- 关卡主题 (Biome) ----------
enum class BiomeType {
    FOREST,
    ICE_DUNGEON,
    VOLCANO
};

// ---------- 房间类型 ----------
enum class RoomType {
    NORMAL,              // 普通战斗房
    ELITE,               // 精英房
    REWARD,              // 奖励房（宝箱）
    SHOP,                // 商店房
    BOSS,                // Boss 房
    START,               // 初始房
    EXIT,                // 小关出口房
    EMPTY                // 空（用于填充地图）
};

// ---------- 门方向 ----------
enum class DoorDirection {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NONE
};

// ---------- 武器类型 ----------
enum class WeaponType {
    ASSAULT_RIFLE = 0,
    SHOTGUN,
    SNIPER_RIFLE,
    ROCKET_LAUNCHER,
    FLAME_THROWER,
    MAGIC_STAFF,
    REBOUND_CROSSBOW,
    VAMPIRE_CODEX,
    BURST_SMG,
    HAND_CANNON,
    FROST_PISTOL,
    SPARK_CARBINE,
    BONE_BOW,
    TOXIC_SPRAYER,
    CHAIN_BLASTER,
    ARCANE_ORB,
    CRYSTAL_LASER,
    THUNDER_RAIL,
    DRAGON_SHOTGUN,
    PLASMA_RIFLE,
    VOID_LAUNCHER,
    STORM_STAFF,
    PHOENIX_CROSSBOW,
    BLOOD_SCYTHE,
    STAR_CANNON,
    GLACIER_MINIGUN,
    METEOR_ROD,
    SUNFIRE_SPEAR,
    SHADOW_CODEX,
    EMPEROR_RIFLE,
    COUNT
};

// ---------- 子弹归属 ----------
enum class BulletFaction {
    PLAYER,
    ENEMY
};

// ---------- 怪物 AI 状态 ----------
enum class EnemyState {
    IDLE,
    PATROL,
    CHASE,
    ATTACK,
    STUNNED,
    DEAD
};

// ---------- Boss 阶段 ----------
enum class BossPhase {
    PHASE_1,
    PHASE_2,
    ENRAGED
};

// ---------- Buff / 天赋枚举 ----------
enum class BuffType {
    HP_BOOST,            // 生命强化
    MP_BOOST,            // 能量强化
    FIRE_SHIELD,         // 火盾
    ICE_SHIELD,          // 冰盾
    BLOODTHIRST,         // 嗜血
    CD_REDUCTION,        // CD缩减
    SPREAD_INCREASE,     // 散弹增加
    DAMAGE_BOOST,        // 伤害强化
    GOLD_BONUS,          // 金币加成
    REVIVE_CHARM,        // 复活十字章
    SHIELD_BOOST,
    MOVE_SPEED_BOOST,
    ARMOR_PLATING,
    CRIT_BOOST,
    FIRE_RATE_BOOST,
    ENERGY_SAVER,
    EXTRA_PROJECTILE,
    POTION_MASTERY,
    BOSS_HUNTER,
    TREASURE_INSTINCT
};

// ---------- 掉落物类型 ----------
enum class DropType {
    GOLD,
    HP_POTION,
    MP_POTION,
    WEAPON_DROP
};

// ---------- 障碍物类型 ----------
enum class ObstacleType {
    TREE,                // 森林：树木
    ICE_PATCH,           // 冰原：冰块
    ICE_BLOCK,           // 冰原：阻挡冰晶
    LAVA_POOL            // 熔岩：岩浆池
};

// ---------- 商店商品 ----------
enum class ShopItemType {
    WEAPON,
    HP_POTION,
    MP_POTION
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
    float x, y;          // 中心坐标
    float halfW, halfH;  // 半宽 / 半高

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

// 圆形碰撞盒（用于某些子弹或特殊碰撞）
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

// ---------- 房间坐标 ----------
struct RoomCoord {
    int col, row;
    RoomCoord() : col(0), row(0) {}
    RoomCoord(int c, int r) : col(c), row(r) {}
    bool operator==(const RoomCoord& o) const { return col == o.col && row == o.row; }
};

// ---------- 工具函数 ----------
inline float DegToRad(float deg) { return deg * (PI / 180.0f); }

inline float RadToDeg(float rad) { return rad * (180.0f / PI); }

// 全局随机数生成器（时间种子，MinGW 上 random_device 是确定性的）
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

// 限制值到范围内
template<typename T>
T Clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// 前向声明关键类
class GameObject;
class Character;
class Player;
class Enemy;
class Boss;
class Bullet;
class Weapon;
class DropItem;
class Obstacle;
class Chest;
class Door;
