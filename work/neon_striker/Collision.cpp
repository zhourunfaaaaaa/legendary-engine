#include "Collision.h"
#include "GameManager.h"
#include "Audio.h"
#include <algorithm>

namespace Collision {

bool CircleCircle(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float distSq = dx * dx + dy * dy;
    float radii = r1 + r2;
    return distSq <= radii * radii;
}

void CheckPlayerBulletEnemy(std::vector<PlayerBullet*>& bullets,
                            std::vector<Enemy*>& enemies,
                            ParticleSystem* particles,
                            GameManager* game) {
    for (auto* bullet : bullets) {
        if (!bullet->IsAlive()) continue;
        for (auto* enemy : enemies) {
            if (!enemy->IsAlive()) continue;
            if (CircleCircle(bullet->GetX(), bullet->GetY(), bullet->GetSize(),
                             enemy->GetX(), enemy->GetY(), enemy->GetSize())) {
                bullet->Kill();

                // 伤害计算
                int dmg = bullet->GetDamage();
                // 护盾减伤
                if (enemy->GetType() == EnemyType::SHIELD && enemy->IsShieldActive()) {
                    dmg = (int)(dmg * (1.0f - enemy->GetShieldDR()));
                    if (enemy->GetHP() - dmg <= (int)(enemy->GetShieldMax() * 0.3f))
                        enemy->SetShieldActive(false);
                }

                enemy->SetHP(enemy->GetHP() - dmg);
                enemy->SetFlashTimer(0.08f);

                // 命中粒子
                particles->Emit(bullet->GetX(), bullet->GetY(), 3, RGB(255, 255, 255),
                                0.15f, 80.0f, 1.5f);
                particles->Emit(bullet->GetX(), bullet->GetY(), 2, bullet->GetColor(),
                                0.2f, 60.0f, 2.0f);
                Audio::GetInstance().PlayHit();

                if (enemy->GetHP() <= 0) {
                    enemy->Kill();
                    game->KillEnemy(enemy);
                }
                break; // 子弹穿透检测下一个敌人
            }
        }
    }
}

bool CheckEnemyBulletPlayer(std::vector<EnemyBullet*>& bullets, Player* player) {
    float pr = player->GetHitRadius();
    for (auto* bullet : bullets) {
        if (!bullet->IsAlive()) continue;
        if (CircleCircle(bullet->GetX(), bullet->GetY(), bullet->GetSize(),
                         player->GetX(), player->GetY(), pr)) {
            bullet->Kill();
            return true;
        }
    }
    return false;
}

bool CheckBossPlayerCollision(Boss* boss, Player* player) {
    if (!boss->IsAlive()) return false;
    float dist = Vector2::Distance(
        Vector2(boss->GetX(), boss->GetY()),
        Vector2(player->GetX(), player->GetY()));
    return dist < (boss->GetSize() * 0.7f + player->GetHitRadius());
}

std::vector<LaserHit> LaserHits(Player* player, const std::vector<Enemy*>& enemies) {
    float lx = player->GetX() + player->GetRecoilX();
    float ly = player->GetY() + player->GetRecoilY() - player->GetSize();
    float halfW = LASER_WIDTH / 2.0f;

    std::vector<LaserHit> hits;
    for (auto* e : enemies) {
        if (!e->IsAlive()) continue;
        float distToLine = std::abs(e->GetX() - lx);
        if (e->GetY() > ly + e->GetSize() || e->GetY() < ly - LASER_REACH - e->GetSize()) continue;
        if (distToLine > halfW + e->GetSize()) continue;
        hits.push_back({ e, ly - e->GetY(), distToLine });
    }
    std::sort(hits.begin(), hits.end(), [](const LaserHit& a, const LaserHit& b) {
        return a.depth < b.depth;
    });
    return hits;
}

bool CheckLaserBossCollision(Player* player, Boss* boss) {
    if (!boss->IsAlive()) return false;
    float lx = player->GetX() + player->GetRecoilX();
    float ly = player->GetY() + player->GetRecoilY() - player->GetSize();
    float halfW = LASER_WIDTH / 2.0f;

    float distToLine = std::abs(boss->GetX() - lx);
    if (distToLine > halfW + boss->GetSize()) return false;
    if (boss->GetY() > ly + boss->GetSize() || boss->GetY() < ly - LASER_REACH - boss->GetSize()) return false;
    return true;
}

} // namespace Collision
