#pragma once

#include "../Core/Common.h"
#include <vector>
#include <memory>
#include <algorithm>

class GameObject;
class Player;
class Enemy;
class Boss;
class Bullet;
class DropItem;
class Obstacle;
class Chest;
class Door;
class Weapon;

// ============================================================
// EntityManager - 统一对象管理器
// 管理所有游戏实体的生命周期、注册、更新、删除
// 使用 std::vector<std::unique_ptr> 实现内存池
// ============================================================
class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    // 注册实体（转移所有权到管理器）
    void RegisterEntity(std::unique_ptr<GameObject> entity);

    // 模板创建实体并返回裸指针（方便后续操作）
    template<typename T, typename... Args>
    T* SpawnEntity(Args&&... args) {
        auto entity = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        T* ptr = entity.get();
        RegisterEntity(std::move(entity));
        return ptr;
    }

    // 标记实体为待删除
    void MarkForDeletion(GameObject* entity);

    // 清理所有待删除实体
    void CleanupDeleted();

    // 清空所有实体
    void ClearAll();

    // 获取特定类型实体列表
    std::vector<Player*>    GetPlayers() const;
    std::vector<Enemy*>     GetEnemies() const;
    std::vector<Boss*>      GetBosses() const;
    std::vector<Bullet*>    GetBullets() const;
    std::vector<Bullet*>    GetPlayerBullets() const;
    std::vector<Bullet*>    GetEnemyBullets() const;
    std::vector<DropItem*>  GetDropItems() const;
    std::vector<Obstacle*>  GetObstacles() const;
    std::vector<Chest*>     GetChests() const;
    std::vector<Door*>      GetDoors() const;

    // 获取所有活跃实体
    const std::vector<std::unique_ptr<GameObject>>& GetAllEntities() const { return m_entities; }

    // 获取数量
    int GetEnemyCount() const;
    int GetBulletCount() const;
    int GetTotalEntityCount() const { return static_cast<int>(m_entities.size()); }

    // 查找最近的敌人
    Enemy* FindNearestEnemy(const Vector2& position, float maxRange = 99999.0f) const;

    // 查找最近的玩家子弹（光剑反弹用）
    Bullet* FindNearestEnemyBullet(const Vector2& position, float maxRange) const;

    // 更新所有实体
    void UpdateAll(float deltaTime);

    // 渲染所有实体（按层级排序）
    void RenderAll();

    // 房间内敌人是否全灭
    bool AreAllEnemiesDead() const;

    // 调试
    int GetActiveBulletCount() const;
    int GetActiveEnemyCount() const;

    // 在指定位置生成爆炸特效（预留接口）
    void SpawnExplosion(const Vector2& position, float radius, int damage, BulletFaction faction);

    // 获取指定范围内的所有敌人
    std::vector<Enemy*> GetEnemiesInRange(const Vector2& position, float radius) const;

private:
    std::vector<std::unique_ptr<GameObject>> m_entities;

    // 按渲染层级排序标记
    bool m_needsSort;
    void SortByRenderLayer();
};
