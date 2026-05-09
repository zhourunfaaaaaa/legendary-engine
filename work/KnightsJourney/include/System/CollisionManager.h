#pragma once

#include "../Core/Common.h"
#include <vector>
#include <functional>

class GameObject;
class EntityManager;

// ============================================================
// CollisionManager - 碰撞检测系统
// 负责所有实体之间的碰撞检测与响应调度
// ============================================================
class CollisionManager {
public:
    CollisionManager();
    ~CollisionManager() = default;

    // 主检测入口：每帧调用，检测所有碰撞对
    void CheckAllCollisions(EntityManager& entityMgr);

    // 分类碰撞检测
    void CheckBulletVsEnemy(EntityManager& entityMgr);
    void CheckBulletVsPlayer(EntityManager& entityMgr);
    void CheckBulletVsBullet(EntityManager& entityMgr);   // 光剑抵消
    void CheckBulletVsObstacle(EntityManager& entityMgr);
    void CheckBulletVsWall(EntityManager& entityMgr);     // 子弹碰到房间墙壁（反弹/消失）
    void CheckPlayerVsObstacle(EntityManager& entityMgr);
    void CheckPlayerVsGroundEffect(EntityManager& entityMgr);
    void CheckPlayerVsDoor(EntityManager& entityMgr);
    void CheckPlayerVsDropItem(EntityManager& entityMgr);
    void CheckPlayerVsChest(EntityManager& entityMgr);
    void CheckEnemyVsPlayer(EntityManager& entityMgr);   // 敌人贴身碰撞伤害
    void CheckEnemyVsObstacle(EntityManager& entityMgr);
    void CheckEnemyVsWall(EntityManager& entityMgr);

    // AABB vs AABB
    static bool AABBIntersects(const AABB& a, const AABB& b);

    // Circle vs AABB
    static bool CircleIntersectsAABB(const CircleCollider& c, const AABB& a);

    // Point vs AABB (鼠标点击检测)
    static bool PointInAABB(float px, float py, const AABB& a);

    // 获取两个 AABB 之间的穿透向量（用于推开重叠实体）
    static Vector2 GetPenetrationVector(const AABB& a, const AABB& b);

    // 反弹方向计算
    static Vector2 CalculateBounceDirection(const Vector2& incoming, const Vector2& normal);
    static Vector2 CalculateBounceDirection(const Vector2& incoming, bool hitVerticalWall);

    // 碰撞回调
    using CollisionCallback = std::function<void(GameObject*, GameObject*)>;
    void RegisterBulletHitCallback(CollisionCallback cb);
    void RegisterPlayerHitCallback(CollisionCallback cb);
    void RegisterEnemyHitCallback(CollisionCallback cb);

    // 调试
    void SetDebugDraw(bool debug) { m_debugDraw = debug; }
    bool IsDebugDraw() const { return m_debugDraw; }

private:
    // 处理子弹击中敌人的逻辑
    void ResolveBulletEnemyHit(class Bullet* bullet, class Enemy* enemy, EntityManager& entityMgr);
    // 处理子弹击中 Boss 的逻辑
    void ResolveBulletEnemyHit(class Bullet* bullet, class Boss* boss, EntityManager& entityMgr);
    // 处理子弹击中玩家的逻辑
    void ResolveBulletPlayerHit(class Bullet* bullet, class Player* player);
    // 处理子弹击中障碍物的逻辑
    void ResolveBulletObstacleHit(class Bullet* bullet, class Obstacle* obstacle);
    // 处理玩家撞门的逻辑
    void ResolvePlayerDoorHit(class Player* player, class Door* door);
    // 处理玩家碰掉落物的逻辑
    void ResolvePlayerDropItemHit(class Player* player, class DropItem* item);
    // 处理玩家开宝箱
    void ResolvePlayerChestHit(class Player* player, class Chest* chest);
    // 处理光剑子弹 vs 敌方子弹
    void ResolveMeleeSlashBlock(class Bullet* meleeSlash, class Bullet* enemyBullet, EntityManager& entityMgr);

    // 检查子弹是否在房间边界内
    static bool IsInsideRoom(const Vector2& pos, const AABB& roomBounds);

    CollisionCallback m_onBulletHit;
    CollisionCallback m_onPlayerHit;
    CollisionCallback m_onEnemyHit;

    bool m_debugDraw;
};

// ============================================================
// 碰撞层级标志 - 用于优化碰撞检测的分组
// ============================================================
enum class CollisionLayer {
    NONE        = 0,
    PLAYER      = 1 << 0,
    ENEMY       = 1 << 1,
    PLAYER_BULLET = 1 << 2,
    ENEMY_BULLET  = 1 << 3,
    OBSTACLE    = 1 << 4,
    DROP_ITEM   = 1 << 5,
    DOOR        = 1 << 6,
    CHEST       = 1 << 7,
    WALL        = 1 << 8,
    ALL         = ~0
};

inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<int>(a) | static_cast<int>(b));
}
inline int operator&(CollisionLayer a, CollisionLayer b) {
    return static_cast<int>(a) & static_cast<int>(b);
}
