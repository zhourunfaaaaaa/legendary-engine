#pragma once

#include "Common.h"
#include "Particle.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Bullet.h"

class GameManager;

// ============================================================
// Collision - 碰撞检测
// ============================================================
namespace Collision {

    // 圆形碰撞
    bool CircleCircle(float x1, float y1, float r1, float x2, float y2, float r2);

    // 玩家子弹 vs 敌人
    void CheckPlayerBulletEnemy(std::vector<PlayerBullet*>& bullets,
                                std::vector<Enemy*>& enemies,
                                ParticleSystem* particles,
                                GameManager* game);

    // 敌人子弹 vs 玩家
    bool CheckEnemyBulletPlayer(std::vector<EnemyBullet*>& bullets,
                                Player* player);

    // BOSS碰撞
    bool CheckBossPlayerCollision(Boss* boss, Player* player);

    // 激光 vs 敌人
    struct LaserHit { Enemy* enemy; float depth; float distToLine; };
    std::vector<LaserHit> LaserHits(Player* player,
                                    const std::vector<Enemy*>& enemies);

    // 激光 vs BOSS
    bool CheckLaserBossCollision(Player* player, Boss* boss);

} // namespace Collision
