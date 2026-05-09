// ============================================================
// EntityManager.cpp - 统一对象管理器实现
// ============================================================

#include "../include/System/EntityManager.h"

#include "../include/Entity/GameObject.h"
#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Entity/Boss.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/DropItem.h"
#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Chest.h"
#include "../include/Entity/Door.h"

#include <graphics.h>

// ============================================================
// 构造 / 析构
// ============================================================
EntityManager::EntityManager()
    : m_needsSort(false) {
    m_entities.reserve(512);  // 预分配内存，减少动态扩容
    printf("[EntityManager] Created. Capacity: 512\n");
}

EntityManager::~EntityManager() {
    ClearAll();
}

// ============================================================
// 注册实体
// ============================================================
void EntityManager::RegisterEntity(std::unique_ptr<GameObject> entity) {
    if (!entity) return;

    entity->SetActive(true);
    m_entities.push_back(std::move(entity));
    m_needsSort = true;
}

// ============================================================
// 标记删除
// ============================================================
void EntityManager::MarkForDeletion(GameObject* entity) {
    if (!entity) return;
    entity->MarkForDeletion();
}

// ============================================================
// 清理死亡/待删除实体（erase-remove 惯用法）
// ============================================================
void EntityManager::CleanupDeleted() {
    auto it = std::remove_if(m_entities.begin(), m_entities.end(),
        [](const std::unique_ptr<GameObject>& entity) {
            return !entity || entity->IsMarkedForDeletion() || !entity->IsActive();
        });

    size_t removed = std::distance(it, m_entities.end());
    m_entities.erase(it, m_entities.end());

    if (removed > 0) {
        // printf("[EntityManager] Cleaned %zu entities. Remaining: %zu\n",
        //        removed, m_entities.size());
    }
}

// ============================================================
// 清空所有实体
// ============================================================
void EntityManager::ClearAll() {
    m_entities.clear();
    m_needsSort = false;
    printf("[EntityManager] All entities cleared.\n");
}

// ============================================================
// 按渲染层级排序
// ============================================================
void EntityManager::SortByRenderLayer() {
    std::sort(m_entities.begin(), m_entities.end(),
        [](const std::unique_ptr<GameObject>& a, const std::unique_ptr<GameObject>& b) {
            if (!a || !b) return false;
            return a->GetRenderLayer() < b->GetRenderLayer();
        });
    m_needsSort = false;
}

// ============================================================
// 更新所有实体
// ============================================================
void EntityManager::UpdateAll(float deltaTime) {
    // 使用下标遍历，避免实体更新中 SpawnEntity 导致 realloc 使迭代器失效
    size_t count = m_entities.size();
    for (size_t i = 0; i < count; ++i) {
        auto& entity = m_entities[i];
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            entity->Update(deltaTime);
        }
    }

    // 移除超时子弹和过期掉落物（标记删除）
    count = m_entities.size();
    for (size_t i = 0; i < count; ++i) {
        auto& entity = m_entities[i];
        if (!entity || !entity->IsActive()) continue;

        if (Bullet* bullet = dynamic_cast<Bullet*>(entity.get())) {
            if (bullet->GetLifetime() <= 0.0f) {
                MarkForDeletion(bullet);
            }
        }

        if (DropItem* item = dynamic_cast<DropItem*>(entity.get())) {
            if (item->IsExpired()) {
                MarkForDeletion(item);
            }
        }
    }
}

// ============================================================
// 渲染所有实体（按层级排序）
// ============================================================
void EntityManager::RenderAll() {
    if (m_needsSort) {
        SortByRenderLayer();
    }

    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            entity->Render();
        }
    }
}

// ============================================================
// 按类型获取实体列表
// 使用 dynamic_cast 进行安全的运行时类型识别
// ============================================================

template<typename T>
static std::vector<T*> FilterEntities(const std::vector<std::unique_ptr<GameObject>>& entities) {
    std::vector<T*> result;
    for (auto& entity : entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            T* casted = dynamic_cast<T*>(entity.get());
            if (casted) {
                result.push_back(casted);
            }
        }
    }
    return result;
}

std::vector<Player*> EntityManager::GetPlayers() const {
    return FilterEntities<Player>(m_entities);
}

std::vector<Enemy*> EntityManager::GetEnemies() const {
    return FilterEntities<Enemy>(m_entities);
}

std::vector<Boss*> EntityManager::GetBosses() const {
    return FilterEntities<Boss>(m_entities);
}

std::vector<Bullet*> EntityManager::GetBullets() const {
    return FilterEntities<Bullet>(m_entities);
}

std::vector<Bullet*> EntityManager::GetPlayerBullets() const {
    std::vector<Bullet*> result;
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Bullet* bullet = dynamic_cast<Bullet*>(entity.get());
            if (bullet && bullet->IsPlayerBullet()) {
                result.push_back(bullet);
            }
        }
    }
    return result;
}

std::vector<Bullet*> EntityManager::GetEnemyBullets() const {
    std::vector<Bullet*> result;
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Bullet* bullet = dynamic_cast<Bullet*>(entity.get());
            if (bullet && bullet->IsEnemyBullet()) {
                result.push_back(bullet);
            }
        }
    }
    return result;
}

std::vector<DropItem*> EntityManager::GetDropItems() const {
    return FilterEntities<DropItem>(m_entities);
}

std::vector<Obstacle*> EntityManager::GetObstacles() const {
    return FilterEntities<Obstacle>(m_entities);
}

std::vector<Chest*> EntityManager::GetChests() const {
    return FilterEntities<Chest>(m_entities);
}

std::vector<Door*> EntityManager::GetDoors() const {
    return FilterEntities<Door>(m_entities);
}

// ============================================================
// 计数
// ============================================================
int EntityManager::GetEnemyCount() const {
    int count = 0;
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            if (dynamic_cast<Enemy*>(entity.get())) {
                ++count;
            }
        }
    }
    return count;
}

int EntityManager::GetBulletCount() const {
    int count = 0;
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            if (dynamic_cast<Bullet*>(entity.get())) {
                ++count;
            }
        }
    }
    return count;
}

int EntityManager::GetActiveBulletCount() const {
    return GetBulletCount();
}

int EntityManager::GetActiveEnemyCount() const {
    return GetEnemyCount();
}

// ============================================================
// 查找最近的敌人
// ============================================================
Enemy* EntityManager::FindNearestEnemy(const Vector2& position, float maxRange) const {
    Enemy* nearest = nullptr;
    float nearestDistSq = maxRange * maxRange;

    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Enemy* enemy = dynamic_cast<Enemy*>(entity.get());
            if (enemy && !enemy->IsDead()) {
                float distSq = Vector2::DistanceSquared(position, enemy->GetPosition());
                if (distSq < nearestDistSq) {
                    nearestDistSq = distSq;
                    nearest = enemy;
                }
            }
        }
    }
    return nearest;
}

// ============================================================
// 查找最近的敌方子弹
// ============================================================
Bullet* EntityManager::FindNearestEnemyBullet(const Vector2& position, float maxRange) const {
    Bullet* nearest = nullptr;
    float nearestDistSq = maxRange * maxRange;

    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Bullet* bullet = dynamic_cast<Bullet*>(entity.get());
            if (bullet && bullet->IsEnemyBullet()) {
                float distSq = Vector2::DistanceSquared(position, bullet->GetPosition());
                if (distSq < nearestDistSq) {
                    nearestDistSq = distSq;
                    nearest = bullet;
                }
            }
        }
    }
    return nearest;
}

// ============================================================
// 敌人是否全灭
// ============================================================
bool EntityManager::AreAllEnemiesDead() const {
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Enemy* enemy = dynamic_cast<Enemy*>(entity.get());
            if (enemy && !enemy->IsDead()) {
                return false;
            }
            Boss* boss = dynamic_cast<Boss*>(entity.get());
            if (boss && !boss->IsDead()) {
                return false;
            }
        }
    }
    return true;
}

// ============================================================
// 获取范围内的敌人
// ============================================================
std::vector<Enemy*> EntityManager::GetEnemiesInRange(const Vector2& position, float radius) const {
    std::vector<Enemy*> result;
    float radiusSq = radius * radius;
    for (auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsMarkedForDeletion()) {
            Enemy* enemy = dynamic_cast<Enemy*>(entity.get());
            if (enemy && !enemy->IsDead()) {
                float distSq = Vector2::DistanceSquared(position, enemy->GetPosition());
                if (distSq <= radiusSq) {
                    result.push_back(enemy);
                }
            }
        }
    }
    return result;
}

// ============================================================
// 生成爆炸效果
// ============================================================
void EntityManager::SpawnExplosion(const Vector2& position, float radius,
                                    int damage, BulletFaction faction) {
    // 爆炸效果：对范围内的敌人/玩家造成伤害
    if (faction == BulletFaction::PLAYER) {
        auto enemies = GetEnemiesInRange(position, radius);
        for (Enemy* enemy : enemies) {
            if (enemy && !enemy->IsDead()) enemy->TakeDamage(damage);
        }
        auto bosses = GetBosses();
        for (Boss* boss : bosses) {
            if (!boss || boss->IsDead()) continue;
            float dist = Vector2::Distance(position, boss->GetPosition());
            if (dist <= radius) {
                boss->TakeDamage(damage);
            }
        }
    } else {
        auto players = GetPlayers();
        for (Player* player : players) {
            if (!player || player->IsDead()) continue;
            float dist = Vector2::Distance(position, player->GetPosition());
            if (dist <= radius) {
                player->TakeDamage(damage);
            }
        }
    }

    // 生成爆炸视觉特效
    ExplosionEffect* effect = SpawnEntity<ExplosionEffect>();
    if (effect) {
        effect->SetPosition(position);
        effect->SetMaxRadius(radius);
    }
}
