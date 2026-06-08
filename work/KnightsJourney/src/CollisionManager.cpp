// ============================================================
// CollisionManager.cpp - 碰撞检测系统
// ============================================================

#include "../include/System/CollisionManager.h"
#include "../include/System/EntityManager.h"
#include "../include/System/SceneManager.h"
#include "../include/System/MapGenerator.h"
#include "../include/System/BuffManager.h"

#include "../include/Entity/GameObject.h"
#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Entity/Boss.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/DropItem.h"
#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Chest.h"
#include "../include/Entity/Door.h"

#include "../include/Core/GameManager.h"

#include <cmath>
#include <cstdio>
#include <windows.h>

namespace {

bool PlayerOwnsWeapon(Player* player, WeaponType type) {
    if (!player) return false;
    for (int i = 0; i < player->GetWeaponCount(); ++i) {
        Weapon* weapon = player->GetWeapon(i);
        if (weapon && weapon->GetType() == type) return true;
    }
    return false;
}

float GetPassiveLifeStealChance(Player* player) {
    if (!player) return 0.0f;
    if (PlayerOwnsWeapon(player, WeaponType::SHADOW_CODEX)) return 0.12f;
    if (PlayerOwnsWeapon(player, WeaponType::BLOOD_SCYTHE)) return 0.10f;
    if (PlayerOwnsWeapon(player, WeaponType::VAMPIRE_CODEX)) return 0.08f;
    return 0.0f;
}

void TryApplyLifeSteal(Bullet* bullet, float passiveScale = 1.0f) {
    if (!bullet || !bullet->IsPlayerBullet()) return;
    Player* player = GameManager::GetInstance().GetPlayer();
    if (!player || player->IsDead()) return;

    float chance = 0.0f;
    if (bullet->HasLifeSteal()) {
        chance = bullet->GetLifeStealChance();
    } else {
        chance = GetPassiveLifeStealChance(player) * passiveScale;
    }

    if (chance > 0.0f && RandomFloat(0.0f, 1.0f) < chance) {
        player->Heal(1);
    }
}

} // namespace

// ============================================================
CollisionManager::CollisionManager()
    : m_debugDraw(false) {
}

// ============================================================
// 主入口：每帧按顺序检查所有碰撞类型
// ============================================================
void CollisionManager::CheckAllCollisions(EntityManager& entityMgr) {
    // 子弹碰撞（优先级最高）
    CheckBulletVsEnemy(entityMgr);
    CheckBulletVsPlayer(entityMgr);
    CheckBulletVsBullet(entityMgr);
    CheckBulletVsObstacle(entityMgr);
    CheckBulletVsWall(entityMgr);

    // 玩家碰撞
    CheckPlayerVsObstacle(entityMgr);
    CheckPlayerVsGroundEffect(entityMgr);  // 地面效果（冰/火）
    CheckPlayerVsDoor(entityMgr);
    CheckPlayerVsDropItem(entityMgr);
    CheckPlayerVsChest(entityMgr);
    CheckDropItemSeparation(entityMgr);

    // 敌人碰撞
    CheckEnemyVsPlayer(entityMgr);
    CheckEnemyVsEnemy(entityMgr);
    CheckEnemyVsObstacle(entityMgr);
    CheckEnemyVsWall(entityMgr);
}

// ============================================================
// 子弹 vs 敌人（核心战斗碰撞）
// ============================================================
void CollisionManager::CheckBulletVsEnemy(EntityManager& entityMgr) {
    auto bullets = entityMgr.GetPlayerBullets();
    auto enemies = entityMgr.GetEnemies();
    auto bosses  = entityMgr.GetBosses();

    for (Bullet* bullet : bullets) {
        if (!bullet || !bullet->IsActive() || bullet->IsMarkedForDeletion())
            continue;

        // 检测 vs 普通敌人
        for (Enemy* enemy : enemies) {
            if (!enemy || !enemy->IsActive() || enemy->IsDead())
                continue;

            if (AABBIntersects(bullet->GetAABB(), enemy->GetAABB())) {
                ResolveBulletEnemyHit(bullet, enemy, entityMgr);
                // 非穿透子弹命中后不再检测后续敌人
                if (!bullet->IsActive() || bullet->IsMarkedForDeletion())
                    break;
            }
        }

        if (!bullet->IsActive() || bullet->IsMarkedForDeletion())
            continue;

        // 检测 vs Boss
        for (Boss* boss : bosses) {
            if (!boss || !boss->IsActive() || boss->IsDead())
                continue;

            if (AABBIntersects(bullet->GetAABB(), boss->GetAABB())) {
                ResolveBulletEnemyHit(bullet, boss, entityMgr);
                if (!bullet->IsActive() || bullet->IsMarkedForDeletion())
                    break;
            }
        }
    }
}

// ============================================================
// 子弹 vs 玩家
// ============================================================
void CollisionManager::CheckBulletVsPlayer(EntityManager& entityMgr) {
    auto bullets = entityMgr.GetEnemyBullets();
    auto players = entityMgr.GetPlayers();

    for (Bullet* bullet : bullets) {
        if (!bullet || !bullet->IsActive() || bullet->IsMarkedForDeletion())
            continue;

        for (Player* player : players) {
            if (!player || !player->IsActive() || player->IsDead())
                continue;

            if (player->IsRollInvincible())
                continue;

            if (AABBIntersects(bullet->GetAABB(), player->GetAABB())) {
                ResolveBulletPlayerHit(bullet, player);
                break;
            }
        }
    }
}

// ============================================================
// 子弹 vs 子弹（光剑刀光抵消敌方子弹）
// ============================================================
void CollisionManager::CheckBulletVsBullet(EntityManager& entityMgr) {
    auto playerBullets = entityMgr.GetPlayerBullets();
    auto enemyBullets  = entityMgr.GetEnemyBullets();

    for (Bullet* pBullet : playerBullets) {
        if (!pBullet || !pBullet->IsActive() || pBullet->IsMarkedForDeletion())
            continue;

        // 只有近战刀光可以抵消敌方子弹
        if (!pBullet->IsMeleeSlash()) continue;

        for (Bullet* eBullet : enemyBullets) {
            if (!eBullet || !eBullet->IsActive() || eBullet->IsMarkedForDeletion())
                continue;

            if (AABBIntersects(pBullet->GetAABB(), eBullet->GetAABB())) {
                ResolveMeleeSlashBlock(pBullet, eBullet, entityMgr);
            }
        }
    }
}

// ============================================================
// 子弹 vs 障碍物
// ============================================================
void CollisionManager::CheckBulletVsObstacle(EntityManager& entityMgr) {
    auto bullets   = entityMgr.GetBullets();
    auto obstacles = entityMgr.GetObstacles();

    for (Bullet* bullet : bullets) {
        if (!bullet || !bullet->IsActive() || bullet->IsMarkedForDeletion())
            continue;

        for (Obstacle* obs : obstacles) {
            if (!obs || !obs->IsActive()) continue;
            if (!obs->BlocksBullets()) continue;

            if (AABBIntersects(bullet->GetAABB(), obs->GetAABB())) {
                ResolveBulletObstacleHit(bullet, obs);
                break;
            }
        }
    }
}

// ============================================================
// 子弹 vs 房间墙壁（反弹子弹、普通子弹销毁）
// ============================================================
void CollisionManager::CheckBulletVsWall(EntityManager& entityMgr) {
    auto bullets = entityMgr.GetBullets();

    for (Bullet* bullet : bullets) {
        if (!bullet || !bullet->IsActive() || bullet->IsMarkedForDeletion())
            continue;

        const float margin = ROOM_WALL_MARGIN;
        bool hitWall = false;
        bool hitVertical = false;

        if (bullet->GetX() < margin) {
            bullet->SetX(margin);
            hitWall = true;
            hitVertical = true;
        } else if (bullet->GetX() > ROOM_WIDTH - margin) {
            bullet->SetX(ROOM_WIDTH - margin);
            hitWall = true;
            hitVertical = true;
        }

        if (bullet->GetY() < margin) {
            bullet->SetY(margin);
            hitWall = true;
            hitVertical = false;
        } else if (bullet->GetY() > ROOM_HEIGHT - margin) {
            bullet->SetY(ROOM_HEIGHT - margin);
            hitWall = true;
            hitVertical = false;
        }

        if (hitWall) {
            if (bullet->IsExplosive()) {
                // 爆炸弹撞墙触发爆炸
                entityMgr.SpawnExplosion(bullet->GetPosition(),
                    bullet->GetExplosionRadius(),
                    bullet->GetDamage(), bullet->GetFaction());
            }
            if (bullet->IsBouncing() && bullet->CanBounce()) {
                // 反弹
                Vector2 newDir = CalculateBounceDirection(
                    bullet->GetDirection(), hitVertical);
                bullet->SetDirection(newDir);
                bullet->IncrementBounce();
                // 反弹后子弹不销毁
            } else if (bullet->IsBouncing() && !bullet->CanBounce()) {
                // 反弹次数用完
                bullet->MarkForDeletion();
            } else {
                // 普通子弹撞墙消失
                bullet->MarkForDeletion();
            }
        }
    }
}

// ============================================================
// 玩家 vs 障碍物（推开重叠）
// ============================================================
void CollisionManager::CheckPlayerVsObstacle(EntityManager& entityMgr) {
    auto players   = entityMgr.GetPlayers();
    auto obstacles = entityMgr.GetObstacles();

    for (Player* player : players) {
        if (!player || !player->IsActive() || player->IsDead()) continue;

        for (Obstacle* obs : obstacles) {
            if (!obs || !obs->IsActive()) continue;
            if (!obs->BlocksMovement()) continue;

            if (AABBIntersects(player->GetAABB(), obs->GetAABB())) {
                Vector2 pen = GetPenetrationVector(player->GetAABB(), obs->GetAABB());
                player->SetPosition(player->GetPosition() + pen);
                player->SyncAABBToPosition();
            }
        }
    }
}

// ============================================================
// 玩家 vs 地面效果（冰面/岩浆）
// ============================================================
void CollisionManager::CheckPlayerVsGroundEffect(EntityManager& entityMgr) {
    auto players   = entityMgr.GetPlayers();
    auto obstacles = entityMgr.GetObstacles();

    for (Player* player : players) {
        if (!player || !player->IsActive() || player->IsDead()) continue;

        for (Obstacle* obs : obstacles) {
            if (!obs || !obs->IsActive()) continue;
            if (!obs->HasGroundEffect()) continue;

            if (AABBIntersects(player->GetAABB(), obs->GetAABB())) {
                obs->ApplyGroundEffect(player, 0.0f);  // deltaTime not used per-call
            }
        }
    }
}

// ============================================================
// 玩家 vs 门（靠近门触发房间切换）
// ============================================================
void CollisionManager::CheckPlayerVsDoor(EntityManager& entityMgr) {
    auto players = entityMgr.GetPlayers();
    auto doors   = entityMgr.GetDoors();

    for (Player* player : players) {
        if (!player || !player->IsActive() || player->IsDead()) continue;

        for (Door* door : doors) {
            if (!door || !door->IsActive()) continue;
            if (!door->IsOpen()) continue;

            if (door->IsPlayerNearby(player->GetPosition())) {
                ResolvePlayerDoorHit(player, door);
                return;  // 一次只处理一个门
            }
        }
    }
}

// ============================================================
// 玩家 vs 掉落物（拾取）
// ============================================================
void CollisionManager::CheckPlayerVsDropItem(EntityManager& entityMgr) {
    auto players = entityMgr.GetPlayers();
    auto items   = entityMgr.GetDropItems();

    for (Player* player : players) {
        if (!player || !player->IsActive() || player->IsDead()) continue;

        for (DropItem* item : items) {
            if (!item || !item->IsActive() || item->IsMarkedForDeletion())
                continue;

            // 使用距离检测（比 AABB 更宽松，便于拾取）
            float dist = Vector2::Distance(player->GetPosition(), item->GetPosition());
            float pickupRange = 42.0f;

            // 磁吸状态下更容易拾取
            if (item->IsMagnetized()) {
                pickupRange = 56.0f;
            }

            if (dist < pickupRange) {
                ResolvePlayerDropItemHit(player, item);
            }
        }
    }
}

// ============================================================
// 玩家 vs 宝箱
// ============================================================
void CollisionManager::CheckPlayerVsChest(EntityManager& entityMgr) {
    auto players = entityMgr.GetPlayers();
    auto chests  = entityMgr.GetChests();

    for (Player* player : players) {
        if (!player || !player->IsActive() || player->IsDead()) continue;

        for (Chest* chest : chests) {
            if (!chest || !chest->IsActive() || chest->IsOpened())
                continue;

            float dist = Vector2::Distance(player->GetPosition(), chest->GetPosition());
            if (dist < 40.0f) {
                // 按 E 键开启
                if (GetAsyncKeyState('E') & 0x8000) {
                    ResolvePlayerChestHit(player, chest);
                }
            }
        }
    }
}

// ============================================================
// 敌人 vs 玩家（贴身碰撞伤害，每 0.5 秒触发一次）
// ============================================================
void CollisionManager::CheckEnemyVsPlayer(EntityManager& entityMgr) {
    auto enemies = entityMgr.GetEnemies();
    auto players = entityMgr.GetPlayers();

    for (Enemy* enemy : enemies) {
        if (!enemy || !enemy->IsActive() || enemy->IsDead()) continue;

        for (Player* player : players) {
            if (!player || !player->IsActive() || player->IsDead()) continue;

            if (AABBIntersects(enemy->GetAABB(), player->GetAABB())) {
                // 推开敌人防止重叠
                Vector2 pen = GetPenetrationVector(enemy->GetAABB(), player->GetAABB());
                enemy->SetPosition(enemy->GetPosition() + pen);
                enemy->SyncAABBToPosition();

                // 接触伤害（Attack 内部处理冷却和距离判断）
                enemy->Attack();
            }
        }
    }
}

// ============================================================
// 敌人 vs 障碍物
// ============================================================
void CollisionManager::CheckEnemyVsEnemy(EntityManager& entityMgr) {
    auto enemies = entityMgr.GetEnemies();

    for (size_t i = 0; i < enemies.size(); ++i) {
        Enemy* a = enemies[i];
        if (!a || !a->IsActive() || a->IsDead()) continue;

        for (size_t j = i + 1; j < enemies.size(); ++j) {
            Enemy* b = enemies[j];
            if (!b || !b->IsActive() || b->IsDead()) continue;
            if (!AABBIntersects(a->GetAABB(), b->GetAABB())) continue;

            Vector2 pen = GetPenetrationVector(a->GetAABB(), b->GetAABB());
            if (pen.LengthSquared() < 0.001f) {
                float side = (a->GetID() < b->GetID()) ? -1.0f : 1.0f;
                pen = Vector2(side * 4.0f, 0.0f);
            }
            a->SetPosition(a->GetPosition() + pen * 0.5f);
            b->SetPosition(b->GetPosition() - pen * 0.5f);
            a->SyncAABBToPosition();
            b->SyncAABBToPosition();
        }
    }
}

void CollisionManager::CheckDropItemSeparation(EntityManager& entityMgr) {
    auto items = entityMgr.GetDropItems();
    const float minDist = 22.0f;
    const float minDistSq = minDist * minDist;

    for (size_t i = 0; i < items.size(); ++i) {
        DropItem* a = items[i];
        if (!a || !a->IsActive() || a->IsMarkedForDeletion()) continue;

        for (size_t j = i + 1; j < items.size(); ++j) {
            DropItem* b = items[j];
            if (!b || !b->IsActive() || b->IsMarkedForDeletion()) continue;

            Vector2 diff = a->GetPosition() - b->GetPosition();
            float distSq = diff.LengthSquared();
            if (distSq >= minDistSq) continue;

            Vector2 dir;
            float dist = std::sqrt(distSq);
            if (dist < 0.001f) {
                float angle = (float)((a->GetID() * 37 + b->GetID() * 19) % 360) * PI / 180.0f;
                dir = Vector2(std::cos(angle), std::sin(angle));
                dist = 1.0f;
            } else {
                dir = diff * (1.0f / dist);
            }

            Vector2 offset = dir * ((minDist - dist) * 0.5f);
            a->SetPosition(a->GetPosition() + offset);
            b->SetPosition(b->GetPosition() - offset);
            a->SetX(Clamp(a->GetX(), ROOM_ENTITY_MARGIN, ROOM_WIDTH - ROOM_ENTITY_MARGIN));
            a->SetY(Clamp(a->GetY(), ROOM_ENTITY_MARGIN, ROOM_HEIGHT - ROOM_ENTITY_MARGIN));
            b->SetX(Clamp(b->GetX(), ROOM_ENTITY_MARGIN, ROOM_WIDTH - ROOM_ENTITY_MARGIN));
            b->SetY(Clamp(b->GetY(), ROOM_ENTITY_MARGIN, ROOM_HEIGHT - ROOM_ENTITY_MARGIN));
            a->SyncAABBToPosition();
            b->SyncAABBToPosition();
        }
    }
}

void CollisionManager::CheckEnemyVsObstacle(EntityManager& entityMgr) {
    auto enemies   = entityMgr.GetEnemies();
    auto obstacles = entityMgr.GetObstacles();

    for (Enemy* enemy : enemies) {
        if (!enemy || !enemy->IsActive() || enemy->IsDead()) continue;

        for (Obstacle* obs : obstacles) {
            if (!obs || !obs->IsActive()) continue;
            if (!obs->BlocksMovement()) continue;

            if (AABBIntersects(enemy->GetAABB(), obs->GetAABB())) {
                Vector2 pen = GetPenetrationVector(enemy->GetAABB(), obs->GetAABB());
                enemy->SetPosition(enemy->GetPosition() + pen);
                enemy->SyncAABBToPosition();
            }
        }
    }
}

// ============================================================
// 敌人 vs 墙壁（不能出房间）
// ============================================================
void CollisionManager::CheckEnemyVsWall(EntityManager& entityMgr) {
    auto enemies = entityMgr.GetEnemies();

    for (Enemy* enemy : enemies) {
        if (!enemy || !enemy->IsActive() || enemy->IsDead()) continue;

        const float margin = ROOM_ENTITY_MARGIN;
        bool clamped = false;

        if (enemy->GetX() < margin)       { enemy->SetX(margin); clamped = true; }
        if (enemy->GetX() > ROOM_WIDTH  - margin) { enemy->SetX(ROOM_WIDTH  - margin); clamped = true; }
        if (enemy->GetY() < margin)       { enemy->SetY(margin); clamped = true; }
        if (enemy->GetY() > ROOM_HEIGHT - margin) { enemy->SetY(ROOM_HEIGHT - margin); clamped = true; }

        if (clamped) {
            enemy->SyncAABBToPosition();
        }
    }
}

// ============================================================
// 碰撞解析函数
// ============================================================

void CollisionManager::ResolveBulletEnemyHit(Bullet* bullet, Enemy* enemy,
                                              EntityManager& entityMgr) {
    if (!bullet || !enemy) return;

    // 穿透子弹防止同一目标多帧重复命中
    if (bullet->IsPiercing() && bullet->HasHitTarget(enemy->GetID())) return;

    // 伤害结算
    int dmg = bullet->GetDamage();
    enemy->TakeDamage(dmg);

    // 追踪已命中目标
    if (bullet->IsPiercing()) bullet->AddHitTarget(enemy->GetID());

    // 子弹特殊效果
    if (bullet->CausesBurning()) {
        if (RandomFloat(0.0f, 1.0f) < bullet->GetBurnChance()) {
            enemy->ApplyBurning(bullet->GetBurnDuration());
        }
    }
    if (bullet->CausesSlow()) {
        enemy->ApplyFrozen(bullet->GetSlowDuration());
    }

    // 吸血判定：魔典类武器自带吸血；拥有吸血魔典时，其他武器也获得较低概率吸血
    TryApplyLifeSteal(bullet, enemy->IsDead() ? 1.8f : 1.0f);

    // 穿透子弹：不销毁，增加穿透计数
    if (bullet->IsPiercing()) {
        bullet->IncrementPierce();
        if (!bullet->CanPierce()) {
            bullet->MarkForDeletion();
        }
    } else {
        // 非穿透子弹命中后消失
        bullet->MarkForDeletion();
    }

    // 爆炸子弹
    if (bullet->IsExplosive()) {
        entityMgr.SpawnExplosion(enemy->GetPosition(),
                                  bullet->GetExplosionRadius(),
                                  bullet->GetDamage() / 2,
                                  bullet->GetFaction());
    }

    // 碰撞回调
    if (m_onBulletHit) {
        m_onBulletHit(bullet, enemy);
    }
}

void CollisionManager::ResolveBulletEnemyHit(Bullet* bullet, Boss* boss,
                                              EntityManager& entityMgr) {
    if (!bullet || !boss) return;

    // 穿透子弹防止同一目标多帧重复命中
    if (bullet->IsPiercing() && bullet->HasHitTarget(boss->GetID())) return;

    int dmg = bullet->GetDamage();
    boss->TakeDamage(dmg);

    // 追踪已命中目标
    if (bullet->IsPiercing()) bullet->AddHitTarget(boss->GetID());

    TryApplyLifeSteal(bullet, 0.65f);

    if (bullet->IsPiercing()) {
        bullet->IncrementPierce();
        if (!bullet->CanPierce()) {
            bullet->MarkForDeletion();
        }
    } else {
        bullet->MarkForDeletion();
    }

    if (bullet->IsExplosive()) {
        entityMgr.SpawnExplosion(boss->GetPosition(),
                                  bullet->GetExplosionRadius(),
                                  bullet->GetDamage() / 2,
                                  bullet->GetFaction());
    }
}

void CollisionManager::ResolveBulletPlayerHit(Bullet* bullet, Player* player) {
    if (!bullet || !player) return;

    player->TakeDamage(bullet->GetDamage());

    // 特殊效果
    BuffManager& buffs = GameManager::GetInstance().GetBuffManager();
    if (bullet->CausesSlow() && !buffs.IsIceImmune()) {
        player->ApplyFrozen(bullet->GetSlowDuration());
    }
    if (bullet->CausesBurning()) {
        player->ApplyBurning(bullet->GetBurnDuration());
    }

    bullet->MarkForDeletion();

    if (m_onPlayerHit) {
        m_onPlayerHit(bullet, player);
    }
}

void CollisionManager::ResolveBulletObstacleHit(Bullet* bullet, Obstacle* obstacle) {
    if (!bullet || !obstacle) return;

    // 爆炸弹碰到障碍物触发爆炸
    if (bullet->IsExplosive()) {
        EntityManager& em = GameManager::GetInstance().GetEntityManager();
        em.SpawnExplosion(bullet->GetPosition(), bullet->GetExplosionRadius(),
                          bullet->GetDamage(), bullet->GetFaction());
    }

    // 反弹子弹碰到障碍物时反射
    if (bullet->IsBouncing() && bullet->CanBounce()) {
        Vector2 normal = (bullet->GetPosition() - obstacle->GetPosition()).Normalized();
        Vector2 bounceDir = CalculateBounceDirection(bullet->GetDirection(), normal);
        bullet->SetDirection(bounceDir);
        bullet->IncrementBounce();
    } else {
        bullet->MarkForDeletion();
    }
}

void CollisionManager::ResolvePlayerDoorHit(Player* player, Door* door) {
    if (!player || !door) return;

    // 通过 SceneManager 切换房间
    GameManager::GetInstance().GetSceneManager().TransitionToRoom(
        GameManager::GetInstance().GetSceneManager().GetMap()->GetCurrentRoomCoord(),
        door->GetTargetRoom()
    );
}

void CollisionManager::ResolvePlayerDropItemHit(Player* player, DropItem* item) {
    if (!player || !item) return;
    item->OnPickup(player);
}

void CollisionManager::ResolvePlayerChestHit(Player* player, Chest* chest) {
    if (!player || !chest) return;

    bool isMimic = chest->Open();

    if (isMimic) {
        // 宝箱怪：生成随机敌人
        SceneManager& sceneMgr = GameManager::GetInstance().GetSceneManager();
        EntityManager& entityMgr = GameManager::GetInstance().GetEntityManager();
        sceneMgr.SpawnEnemy(chest->GetPosition(), entityMgr, false);
        printf("[Collision] Mimic chest spawned an enemy!\n");
    } else {
        // 直接给玩家内容物，不再通过掉落物中间步骤
        switch (chest->GetContentType()) {
            case DropType::GOLD: {
                GameManager::GetInstance().AddGold(chest->GetContentValue());
                printf("[Chest] Player received %d gold\n", chest->GetContentValue());
                break;
            }
            case DropType::HP_POTION: {
                player->Heal(chest->GetContentValue());
                printf("[Chest] Player received HP potion: +%d\n", chest->GetContentValue());
                break;
            }
            case DropType::MP_POTION: {
                player->SetMP(player->GetMP() + chest->GetContentValue());
                printf("[Chest] Player received MP potion: +%d\n", chest->GetContentValue());
                break;
            }
            case DropType::WEAPON_DROP: {
                if (!player->EquipWeapon(chest->GetContainedWeapon())) {
                    // 武器槽满，触发选择界面
                    GameManager::GetInstance().SetState(GameState::WEAPON_SELECT);
                }
                break;
            }
        }
    }
}

void CollisionManager::ResolveMeleeSlashBlock(Bullet* meleeSlash, Bullet* enemyBullet,
                                               EntityManager& entityMgr) {
    if (!meleeSlash || !enemyBullet) return;

    // 抵消敌方子弹（近战武器已移除，直接删除）
    (void)entityMgr;
    enemyBullet->MarkForDeletion();
}

// ============================================================
// 静态碰撞检测工具
// ============================================================

bool CollisionManager::AABBIntersects(const AABB& a, const AABB& b) {
    return !(a.Right()  < b.Left()  ||
             a.Left()   > b.Right() ||
             a.Bottom() < b.Top()   ||
             a.Top()    > b.Bottom());
}

bool CollisionManager::CircleIntersectsAABB(const CircleCollider& c, const AABB& a) {
    float closestX = (c.x < a.Left())   ? a.Left()   :
                     (c.x > a.Right())  ? a.Right()  : c.x;
    float closestY = (c.y < a.Top())    ? a.Top()    :
                     (c.y > a.Bottom()) ? a.Bottom() : c.y;

    float dx = c.x - closestX;
    float dy = c.y - closestY;
    return (dx * dx + dy * dy) <= (c.radius * c.radius);
}

bool CollisionManager::PointInAABB(float px, float py, const AABB& a) {
    return px >= a.Left()  && px <= a.Right() &&
           py >= a.Top()   && py <= a.Bottom();
}

// ============================================================
// 穿透向量（用于推开重叠的实体）
// ============================================================
Vector2 CollisionManager::GetPenetrationVector(const AABB& a, const AABB& b) {
    float overlapLeft   = a.Right()  - b.Left();
    float overlapRight  = b.Right()  - a.Left();
    float overlapTop    = a.Bottom() - b.Top();
    float overlapBottom = b.Bottom() - a.Top();

    // 取最小重叠方向
    float minX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
    float minY = (overlapTop < overlapBottom) ? -overlapTop : overlapBottom;

    if (std::fabs(minX) < std::fabs(minY)) {
        return Vector2(minX, 0.0f);
    } else {
        return Vector2(0.0f, minY);
    }
}

// ============================================================
// 反弹方向计算
// ============================================================
Vector2 CollisionManager::CalculateBounceDirection(const Vector2& incoming,
                                                    const Vector2& normal) {
    float dot = incoming.Dot(normal);
    Vector2 reflected = incoming - normal * (2.0f * dot);
    return reflected.Normalized();
}

Vector2 CollisionManager::CalculateBounceDirection(const Vector2& incoming,
                                                    bool hitVerticalWall) {
    if (hitVerticalWall) {
        return Vector2(-incoming.x, incoming.y);
    } else {
        return Vector2(incoming.x, -incoming.y);
    }
}

// ============================================================
// 房间边界检查
// ============================================================
bool CollisionManager::IsInsideRoom(const Vector2& pos, const AABB& roomBounds) {
    return pos.x >= roomBounds.Left()  && pos.x <= roomBounds.Right() &&
           pos.y >= roomBounds.Top()   && pos.y <= roomBounds.Bottom();
}

// ============================================================
// 碰撞回调注册
// ============================================================
void CollisionManager::RegisterBulletHitCallback(CollisionCallback cb) {
    m_onBulletHit = cb;
}
void CollisionManager::RegisterPlayerHitCallback(CollisionCallback cb) {
    m_onPlayerHit = cb;
}
void CollisionManager::RegisterEnemyHitCallback(CollisionCallback cb) {
    m_onEnemyHit = cb;
}
