#include "GameManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Bullet.h"
#include "Particle.h"
#include "Input.h"
#include "Renderer.h"
#include "Audio.h"
#include "Collision.h"
#include <graphics.h>

// ============================================================
// GameManager 单例
// ============================================================
GameManager& GameManager::GetInstance() {
    static GameManager instance;
    return instance;
}

void GameManager::Init() {
    m_gameState = GameState::MAIN_MENU;
    m_player = new Player(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.75f);
    m_particles = new ParticleSystem();
}

void GameManager::Shutdown() {
    // 清理Boss
    if (m_boss) { delete m_boss; m_boss = nullptr; }
    if (m_infiniteBoss) { delete m_infiniteBoss; m_infiniteBoss = nullptr; }
    // 清理敌人
    for (auto* e : m_enemies) delete e;
    m_enemies.clear();
    // 清理子弹
    for (auto* b : m_playerBullets) delete b;
    m_playerBullets.clear();
    for (auto* b : m_enemyBullets) delete b;
    m_enemyBullets.clear();
    // 清理玩家
    if (m_player) { delete m_player; m_player = nullptr; }
    // 清理粒子
    if (m_particles) { delete m_particles; m_particles = nullptr; }
}

// ============================================================
// ProcessFrame - 主帧处理
// ============================================================
void GameManager::ProcessFrame(float rawDt) {
    float clampedDt = (std::min)(rawDt, MAX_FRAME_TIME);
    if (clampedDt <= 0.0f) clampedDt = 0.016f;

    // 玩家移动不受子弹时间影响
    m_playerDt = (std::min)(clampedDt, MAX_DELTA_TIME);
    // 游戏逻辑受慢动作影响
    m_deltaTime = (std::min)(clampedDt, MAX_DELTA_TIME) * m_slowMotion;

    // ★ ESC 切换暂停 (PLAYING ↔ PAUSED)
    if (m_gameState == GameState::PLAYING && Input::GetInstance().WasPressed(VK_ESCAPE)) {
        m_gameState = GameState::PAUSED;
        Input::GetInstance().ResetInputState(); // 防止穿透
        return;
    }
    if (m_gameState == GameState::PAUSED && Input::GetInstance().WasPressed(VK_ESCAPE)) {
        m_gameState = GameState::PLAYING;
        Input::GetInstance().ResetInputState();
        return;
    }

    switch (m_gameState) {
        case GameState::MAIN_MENU:
            m_gameTime += m_deltaTime;
            Renderer::GetInstance().UpdateBackground(m_deltaTime);
            Renderer::GetInstance().DrawMainMenu();
            FlushBatchDraw();
            break;

        case GameState::INTRO:
            m_gameTime += m_deltaTime;
            Renderer::GetInstance().UpdateIntroAnimation(m_deltaTime);
            Renderer::GetInstance().Render();
            break;

        case GameState::PAUSED:
            // 冻结游戏逻辑，继续渲染
            Renderer::GetInstance().Render();
            break;

        case GameState::PLAYING:
            m_gameTime += m_deltaTime;
            if (m_bossMode && m_boss) {
                UpdateBossMode(m_deltaTime, m_playerDt);
            } else {
                UpdateInfiniteMode(m_deltaTime, m_playerDt);
            }
            break;

        case GameState::GAME_OVER:
            m_gameTime += m_deltaTime;
            Renderer::GetInstance().UpdateBackground(m_deltaTime);
            Renderer::GetInstance().DrawGameOverScreen();
            FlushBatchDraw();
            break;

        case GameState::VICTORY:
            m_gameTime += m_deltaTime;
            Renderer::GetInstance().UpdateBackground(m_deltaTime);
            Renderer::GetInstance().DrawVictoryScreen();
            FlushBatchDraw();
            break;

        case GameState::DEFEAT:
            m_gameTime += m_deltaTime;
            Renderer::GetInstance().UpdateBackground(m_deltaTime);
            Renderer::GetInstance().DrawDefeatScreen();
            FlushBatchDraw();
            break;

        default:
            break;
    }
}

// ============================================================
// UpdateInfiniteMode - 无限模式
// ============================================================
void GameManager::UpdateInfiniteMode(float dt, float playerDt) {
    // 子弹时间
    if (Input::GetInstance().IsBulletTime()) {
        if (!m_btSoundActive) {
            m_btSoundActive = true;
            Audio::GetInstance().PlayTimeWarpOn();
        }
        m_slowMotion = SLOW_MOTION_FACTOR;
    } else if (!m_ultimateActive) {
        if (m_btSoundActive) {
            m_btSoundActive = false;
            Audio::GetInstance().PlayTimeWarpOff();
        }
        m_slowMotion = (std::min)(1.0f, m_slowMotion + dt * SLOW_MOTION_RECOVER);
    }

    // 武器切换
    if (Input::GetInstance().WasPressed('1')) m_player->SetWeaponType(WeaponType::BEAM);
    if (Input::GetInstance().WasPressed('2')) m_player->SetWeaponType(WeaponType::PULSE);
    if (Input::GetInstance().WasPressed('3')) m_player->SetWeaponType(WeaponType::SCATTER);

    // 大招
    if (Input::GetInstance().WasPressed('E')) ActivateUltimate();

    // 滤镜切换
    if (Input::GetInstance().WasPressed('F')) ToggleFilter();

    // 更新大招计时
    if (m_ultimateActive) {
        m_ultimateTimer -= dt;
        if (m_ultimateTimer <= 0.0f) {
            m_ultimateActive = false;
            m_slowMotion = 1.0f;
        }
    }

    // 更新玩家
    m_player->Update(playerDt);

    // 射击 (无声)
    FireWeapon(dt);

    // 激光
    UpdateLaser(dt);

    // 阶段切换动画
    if (m_transTimer > 0.0f) m_transTimer -= dt;
    if (m_killFlashTimer > 0.0f) m_killFlashTimer -= dt;

    // 通用特效
    UpdateCommonEffects(dt);

    // 更新子弹
    UpdateBullets(dt);

    // 更新敌人
    UpdateEnemies(dt);

    // 生成敌人
    SpawnEnemies();

    // 敌人射击
    for (auto* e : m_enemies) {
        if (!e->IsAlive()) continue;
        e->SetShootTimer(e->GetShootTimer() - dt);
        if (e->GetShootTimer() <= 0.0f) {
            e->SetShootTimer(e->GetShootInterval());
            EnemyShoot(e);
        }
    }

    // ★ 无限Boss攻击
    if (m_infiniteBoss && m_infiniteBoss->IsAlive()) {
        m_infiniteBoss->Update(dt);
        BossShoot(m_infiniteBoss);
        // 玩家子弹 vs 无限Boss
        for (auto* b : m_playerBullets) {
            if (!b->IsAlive()) continue;
            float dx = b->GetX() - m_infiniteBoss->GetX();
            float dy = b->GetY() - m_infiniteBoss->GetY();
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < b->GetSize() + m_infiniteBoss->GetSize() * 0.7f) {
                b->Kill();
                m_infiniteBoss->SetHP(m_infiniteBoss->GetHP() - b->GetDamage());
                m_infiniteBoss->SetFlashTimer(0.06f);
                m_particles->Emit(b->GetX(), b->GetY(), 3, RGB(255,255,255), 0.15f, 80.0f, 1.5f);
                Audio::GetInstance().PlayHit();
            }
        }
    }

    // 碰撞检测
    HandleCollisions();

    // 更新粒子
    m_particles->Update(dt);

    // 更新背景
    Renderer::GetInstance().UpdateBackground(dt);

    // 渲染
    Renderer::GetInstance().Render();
}

// ============================================================
// UpdateBossMode - BOSS挑战模式
// ============================================================
void GameManager::UpdateBossMode(float dt, float playerDt) {
    m_bossGameTimer += dt;

    // 子弹时间
    if (Input::GetInstance().IsBulletTime()) {
        if (!m_btSoundActive) { m_btSoundActive = true; Audio::GetInstance().PlayTimeWarpOn(); }
        m_slowMotion = SLOW_MOTION_FACTOR;
    } else if (!m_ultimateActive) {
        if (m_btSoundActive) { m_btSoundActive = false; Audio::GetInstance().PlayTimeWarpOff(); }
        m_slowMotion = (std::min)(1.0f, m_slowMotion + dt * SLOW_MOTION_RECOVER);
    }

    // 武器切换
    if (Input::GetInstance().WasPressed('1')) m_player->SetWeaponType(WeaponType::BEAM);
    if (Input::GetInstance().WasPressed('2')) m_player->SetWeaponType(WeaponType::PULSE);
    if (Input::GetInstance().WasPressed('3')) m_player->SetWeaponType(WeaponType::SCATTER);

    // 大招
    if (Input::GetInstance().WasPressed('E')) ActivateUltimate();

    // 更新大招计时
    if (m_ultimateActive) {
        m_ultimateTimer -= dt;
        if (m_ultimateTimer <= 0.0f) { m_ultimateActive = false; m_slowMotion = 1.0f; }
    }

    // ★ 更新玩家
    m_player->Update(playerDt);

    // 射击 (无声)
    FireWeapon(dt);

    // 激光
    UpdateLaser(dt);

    // 阶段切换动画
    if (m_transTimer > 0.0f) m_transTimer -= dt;
    if (m_killFlashTimer > 0.0f) m_killFlashTimer -= dt;

    // 通用特效
    UpdateCommonEffects(dt);

    // 更新子弹
    UpdateBullets(dt);

    // ★ BOSS更新
    m_boss->Update(dt);

    // ★ BOSS攻击
    BossShoot(m_boss);

    // ★ BOSS子弹碰撞玩家
    if (m_player->GetInvincible() <= 0.0f) {
        for (auto* b : m_enemyBullets) {
            if (!b->IsAlive()) continue;
            float dx = b->GetX() - m_player->GetX();
            float dy = b->GetY() - m_player->GetY();
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < b->GetSize() + m_player->GetHitRadius()) {
                b->Kill();
                m_player->TakeDamage(10);
                if (m_player->GetHP() <= 0) { Defeat(); return; }
            }
        }
    }

    // ★ BOSS碰撞玩家
    if (m_player->GetInvincible() <= 0.0f) {
        if (Collision::CheckBossPlayerCollision(m_boss, m_player)) {
            m_player->TakeDamage(15);
            if (m_player->GetHP() <= 0) { Defeat(); return; }
        }
    }

    // ★ 玩家子弹 vs BOSS
    for (auto* b : m_playerBullets) {
        if (!b->IsAlive()) continue;
        float dx = b->GetX() - m_boss->GetX();
        float dy = b->GetY() - m_boss->GetY();
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < b->GetSize() + m_boss->GetSize() * 0.7f) {
            b->Kill();
            m_boss->SetHP(m_boss->GetHP() - b->GetDamage());
            m_bossDamageDealt += b->GetDamage();
            m_boss->SetFlashTimer(0.06f);
            m_particles->Emit(b->GetX(), b->GetY(), 3, RGB(255,255,255), 0.15f, 80.0f, 1.5f);
            Audio::GetInstance().PlayHit();
        }
    }

    // BOSS阶段解锁 (时间)
    if (m_bossGameTimer >= BOSS_STAGE2_TIME && m_currentStage < 1) {
        SetCurrentStage(1); m_transTimer = 1.0f; m_transStage = 1;
    }
    if (m_bossGameTimer >= BOSS_STAGE3_TIME && m_currentStage < 2) {
        SetCurrentStage(2); m_transTimer = 1.2f; m_transStage = 2;
    }

    // 更新粒子
    m_particles->Update(dt);

    // 更新背景
    Renderer::GetInstance().UpdateBackground(dt);

    // BOSS血条
    Renderer::GetInstance().ShowBossHPBar(true);
    m_bossPhase = m_boss->GetBossPhase();

    // 渲染
    Renderer::GetInstance().Render();

    // BOSS死亡
    if (!m_boss->IsAlive()) {
        m_bossFinishTime = m_bossGameTimer;
        Victory();
    }

    // 玩家死亡
    if (m_player->GetHP() <= 0) {
        Defeat();
    }
}

// ============================================================
// FireWeapon - 武器射击
// ============================================================
void GameManager::FireWeapon(float dt) {
    if (!m_player->CanShoot()) return;
    m_player->Shoot();

    float cx = m_player->GetX() + m_player->GetRecoilX();
    float cy = m_player->GetY() + m_player->GetRecoilY() - m_player->GetSize();

    // 阶段3: 激光接管
    if (m_currentStage >= 2) {
        m_laserActive = true;
        return;
    }

    float dmgMult = (m_currentStage >= 1) ? 0.5f : 1.0f;
    WeaponType wt = m_player->GetWeaponType();

    auto fireBeam = [&](float ox, float oy, float mul) {
        auto* b = new PlayerBullet(cx + ox, cy + oy, 0, -900,
                                    (int)(15.0f * mul), RGB(0, 255, 255),
                                    2.5f * (mul < 1.0f ? 1.4f : 1.0f));
        m_playerBullets.push_back(b);
        m_particles->Emit(cx + ox, cy + oy, 3, RGB(0, 255, 255), 0.15f, 50.0f, 1.5f);
    };

    auto firePulse = [&](float ox, float oy, float mul) {
        auto* b = new PlayerBullet(cx + ox, cy + oy, 0, -700,
                                    (int)(35.0f * mul), RGB(0, 170, 255),
                                    5.0f * (mul < 1.0f ? 1.3f : 1.0f));
        m_playerBullets.push_back(b);
        m_particles->Emit(cx + ox, cy + oy, 8, RGB(0, 170, 255), 0.2f, 80.0f, 3.0f);
        m_particles->EmitRing(cx + ox, cy + oy, 5, RGB(255, 255, 255), 0.1f, 30.0f, 1.0f);
    };

    auto fireScatter = [&](float ox, float oy, float mul) {
        COLORREF colors[] = { RGB(0,255,255), RGB(0,204,255), RGB(0,170,255), RGB(0,136,255), RGB(255,255,255) };
        for (int i = -2; i <= 2; i++) {
            float angle = -PI / 2.0f + i * 0.25f;
            float speed = 600.0f;
            auto* b = new PlayerBullet(cx + ox, cy + oy,
                                        std::cos(angle) * speed, std::sin(angle) * speed,
                                        (int)(12.0f * mul), colors[i + 2],
                                        2.0f * (mul < 1.0f ? 1.3f : 1.0f));
            m_playerBullets.push_back(b);
        }
        m_particles->Emit(cx + ox, cy + oy, 10, RGB(255, 255, 0), 0.15f, 60.0f, 2.0f);
    };

    if (wt == WeaponType::PULSE) firePulse(0, 0, dmgMult);
    else if (wt == WeaponType::SCATTER) fireScatter(0, 0, dmgMult);
    else fireBeam(0, 0, dmgMult);

    // 阶段2: 第二发
    if (m_currentStage >= 1) {
        float ox = 6.0f;
        if (wt == WeaponType::PULSE) firePulse(-ox, 0, dmgMult);
        else if (wt == WeaponType::SCATTER) fireScatter(0, -4, dmgMult);
        else fireBeam(ox, 0, dmgMult);
    }
}

// ============================================================
// ActivateUltimate - 大招
// ============================================================
void GameManager::ActivateUltimate() {
    if (!m_player->UseUltimate()) return;

    m_ultimateActive = true;
    m_ultimateTimer = ULTIMATE_DURATION;
    Audio::GetInstance().PlayUltimate();
    AddScreenShake(12.0f, 0.4f);
    AddScreenFlash(0.8f);

    // 清除敌人子弹
    for (auto* b : m_enemyBullets) {
        if (b->IsAlive()) {
            m_particles->Emit(b->GetX(), b->GetY(), 3, b->GetColor(), 0.3f, 100.0f, 2.0f);
            b->Kill();
        }
    }

    // 伤害所有敌人
    for (auto* e : m_enemies) {
        if (!e->IsAlive()) continue;
        int dmg = 200;
        if (m_currentStage >= 2) {
            dmg = 200 + (int)(e->GetMaxHP() * 0.35f);
        }
        e->SetHP(e->GetHP() - dmg);
        e->SetFlashTimer(0.2f);
        m_particles->Emit(e->GetX(), e->GetY(), 10, RGB(255, 255, 255), 0.3f, 150.0f, 3.0f);
        if (e->GetHP() <= 0) {
            e->Kill();
            KillEnemy(e);
        }
    }

    // BOSS模式
    if (m_bossMode && m_boss && m_boss->IsAlive()) {
        float pct = 0.08f + RandomFloat(0.0f, 0.04f);
        int dmg = (int)(m_boss->GetMaxHP() * pct);
        m_boss->SetHP(m_boss->GetHP() - dmg);
        m_bossDamageDealt += dmg;
        m_boss->SetFlashTimer(0.2f);
        m_particles->Emit(m_boss->GetX(), m_boss->GetY(), 15, RGB(255,255,255), 0.3f, 200.0f, 3.0f);
        m_particles->Emit(m_boss->GetX(), m_boss->GetY(), 10, RGB(255,215,0), 0.25f, 150.0f, 2.5f);
    }

    // 无限模式Boss
    if (!m_bossMode && m_infiniteBoss && m_infiniteBoss->IsAlive()) {
        float pct = 0.08f + RandomFloat(0.0f, 0.04f);
        int dmg = (int)(m_infiniteBoss->GetMaxHP() * pct);
        m_infiniteBoss->SetHP(m_infiniteBoss->GetHP() - dmg);
        m_infiniteBoss->SetFlashTimer(0.2f);
        m_particles->Emit(m_infiniteBoss->GetX(), m_infiniteBoss->GetY(), 15, RGB(255,255,255), 0.3f, 200.0f, 3.0f);
    }
}

// ============================================================
// UpdateLaser - 阶段3激光系统
// ============================================================
void GameManager::UpdateLaser(float dt) {
    if (m_currentStage < 2 || m_gameState != GameState::PLAYING) {
        m_laserActive = false;
        return;
    }
    m_laserActive = true;

    // 持续后座力抖动
    m_player->SetRecoilX(m_player->GetRecoilX() + (RandomFloat(0.0f, 1.0f) - 0.5f) * 0.6f);
    m_player->SetRecoilY(m_player->GetRecoilY() - 0.3f);

    // tick伤害
    m_laserTickTimer -= dt;
    if (m_laserTickTimer <= 0.0f) {
        m_laserTickTimer = LASER_TICK_GAP;

        // BOSS模式: 激光命中BOSS
        if (m_bossMode && m_boss && m_boss->IsAlive()) {
            if (Collision::CheckLaserBossCollision(m_player, m_boss)) {
                m_boss->SetHP(m_boss->GetHP() - (int)LASER_DMG_TICK);
                m_bossDamageDealt += (int)LASER_DMG_TICK;
                m_boss->SetFlashTimer(0.06f);
                m_particles->Emit(m_boss->GetX(), m_boss->GetY(), 4, RGB(255,255,255), 0.15f, 100.0f, 2.0f);
                m_particles->Emit(m_boss->GetX(), m_boss->GetY(), 3, RGB(0,255,255), 0.2f, 80.0f, 2.5f);
                Audio::GetInstance().PlayHit();
                if (m_boss->GetHP() <= 0) {
                    m_boss->Kill();
                    m_bossFinishTime = m_bossGameTimer;
                    Victory();
                    return;
                }
            }
        }

        // 无限Boss
        if (m_infiniteBoss && m_infiniteBoss->IsAlive()) {
            if (Collision::CheckLaserBossCollision(m_player, m_infiniteBoss)) {
                m_infiniteBoss->SetHP(m_infiniteBoss->GetHP() - (int)LASER_DMG_TICK);
                m_infiniteBoss->SetFlashTimer(0.06f);
                m_particles->Emit(m_infiniteBoss->GetX(), m_infiniteBoss->GetY(), 4, RGB(255,255,255), 0.15f, 100.0f, 2.0f);
                Audio::GetInstance().PlayHit();
                if (m_infiniteBoss->GetHP() <= 0) {
                    m_infiniteBoss->Kill();
                    // 掉落处理
                    m_particles->EmitRing(m_infiniteBoss->GetX(), m_infiniteBoss->GetY(), 30, RGB(255,215,0), 0.5f, 200.0f, 5.0f);
                    m_nextInfiniteBossKill = m_totalKills + NEXT_INFINITE_BOSS;
                    m_infiniteBossCount++;
                    Audio::GetInstance().PlayInfiniteBossDeath();
                    return;
                }
            }
        }

        // 激光命中普通敌人
        auto hits = Collision::LaserHits(m_player, m_enemies);
        if (hits.empty()) return;

        bool stopped = false;
        for (auto& h : hits) {
            if (stopped) break;
            Enemy* e = h.enemy;
            EnemyType et = e->GetType();

            if (et == EnemyType::NORMAL || et == EnemyType::LARGE || et == EnemyType::SHIELD) {
                if (et == EnemyType::SHIELD && e->IsShieldActive()) {
                    e->SetHP(e->GetHP() - (int)(LASER_DMG_TICK * 0.4f));
                    if (e->GetHP() <= (int)(e->GetShieldMax() * 0.2f)) e->SetShieldActive(false);
                } else {
                    e->SetHP(e->GetHP() - (int)LASER_DMG_TICK);
                }
                e->SetFlashTimer(0.06f);
                m_particles->Emit(e->GetX(), e->GetY(), 4, RGB(255,255,255), 0.15f, 100.0f, 2.0f);
                m_particles->Emit(e->GetX(), e->GetY(), 3, RGB(0,255,255), 0.2f, 80.0f, 2.5f);
                Audio::GetInstance().PlayHit();
                if (e->GetHP() <= 0) {
                    e->Kill();
                    KillEnemy(e);
                }
                stopped = true;
                continue;
            }

            // 小敌穿透
            e->SetHP(e->GetHP() - (int)LASER_DMG_TICK);
            e->SetFlashTimer(0.06f);
            m_particles->Emit(e->GetX(), e->GetY(), 3, RGB(255,255,255), 0.12f, 80.0f, 1.5f);
            m_particles->Emit(e->GetX(), e->GetY(), 2, RGB(0,255,255), 0.15f, 60.0f, 2.0f);
            Audio::GetInstance().PlayHit();
            if (e->GetHP() <= 0) {
                e->Kill();
                KillEnemy(e);
            }
        }

        if (hits.size() >= 3) {
            AddScreenShake(1.0f + hits.size() * 0.5f, 0.08f);
        }
    }
}

// ============================================================
// KillEnemy - 击杀敌人
// ============================================================
void GameManager::KillEnemy(Enemy* enemy) {
    // 连击
    m_comboCount++;
    m_comboTimer = COMBO_TIMEOUT;
    if (m_comboCount > m_maxCombo) m_maxCombo = m_comboCount;
    int comboBonus = (std::min)(m_comboCount, 50);

    float multiplier = 1.0f + (m_comboCount / 10) * 0.5f;
    int pts = (int)(enemy->GetScoreValue() * multiplier);
    m_score += pts;

    // 能量
    int energyGain = 5 + comboBonus;
    m_player->AddEnergy(energyGain);

    // 粒子
    m_particles->EmitRing(enemy->GetX(), enemy->GetY(), 12, enemy->GetColor(), 0.3f, 150.0f, 3.0f);
    Audio::GetInstance().PlayExplosion(0.5f);

    // 连击音效
    if (m_comboCount % 10 == 0) {
        Audio::GetInstance().PlayCombo(m_comboCount / 10);
    }

    // 击杀计数
    m_totalKills++;
    m_killFlashTimer = 0.15f; // 击杀跳动

    // 阶段检测
    UpdateStage();

    // 分裂敌人
    if (enemy->GetType() == EnemyType::SPLITTER && !enemy->HasSplit()) {
        enemy->SetHasSplit(true);
        int count = enemy->GetSplitCount();
        for (int i = 0; i < count; i++) {
            float ax = enemy->GetX() + RandomFloat(-20.0f, 20.0f);
            float ay = enemy->GetY() + RandomFloat(-20.0f, 20.0f);
            Enemy* small = new Enemy(ax, ay, EnemyType::SMALL);
            small->SetHP(small->GetHP() / 2);
            m_enemies.push_back(small);
        }
    }

    // 阶段3回血
    if (m_currentStage >= 2 && !m_bossMode) {
        int heal = (int)(m_player->GetMaxHP() * LIFESTEAL_PCT);
        m_player->SetHP((std::min)(m_player->GetMaxHP(), m_player->GetHP() + heal));
        if (m_totalKills % 5 == 0) {
            m_healFlashTimer = 0.3f;
        }
    }

    // 无限Boss生成
    if (!m_bossMode && m_totalKills >= m_nextInfiniteBossKill && !m_infiniteBoss) {
        SpawnInfiniteBoss();
    }
}

void GameManager::SpawnInfiniteBoss() {
    if (m_infiniteBoss) { delete m_infiniteBoss; m_infiniteBoss = nullptr; }
    m_infiniteBoss = new Boss(WINDOW_WIDTH / 2.0f, 80.0f, INFINITE_BOSS_HP, true);
    Audio::GetInstance().PlayInfiniteBossAppear();
    AddScreenShake(8.0f, 0.5f);
    m_particles->EmitRing(WINDOW_WIDTH / 2.0f, 80.0f, 30, RGB(255, 0, 0), 0.5f, 200.0f, 5.0f);
}

// ============================================================
// UpdateStage - 阶段成长
// ============================================================
void GameManager::UpdateStage() {
    if (m_bossMode) return; // Boss模式用时间解锁

    if (m_totalKills >= STAGE3_KILLS && m_currentStage < 2) {
        m_currentStage = 2;
        m_transTimer = 1.2f;
        m_transStage = 2;
    } else if (m_totalKills >= STAGE2_KILLS && m_currentStage < 1) {
        m_currentStage = 1;
        m_transTimer = 1.0f;
        m_transStage = 1;
    }
}

// ============================================================
// SpawnEnemies
// ============================================================
void GameManager::SpawnEnemies() {
    m_spawnTimer += m_deltaTime;

    float spawnRate = SPAWN_RATE_BASE + m_wave * SPAWN_RATE_WAVE;
    int maxConcurrent = MAX_CONCURRENT_BASE + m_wave * MAX_CONCURRENT_WAVE;

    int activeCount = 0;
    for (auto* e : m_enemies) if (e->IsAlive()) activeCount++;

    // 防卡死：超过3秒无怪强制生成
    if (activeCount == 0 && m_spawnTimer > 3.0f) {
        int count = 1 + RandomInt(0, 1);
        for (int i = 0; i < count; i++) {
            EnemyType t = RandomFloat(0.0f, 1.0f) < 0.5f ? EnemyType::SMALL : EnemyType::NORMAL;
            float x = 50.0f + RandomFloat(0.0f, (float)(WINDOW_WIDTH - 100));
            m_enemies.push_back(new Enemy(x, -30.0f, t));
        }
        m_spawnTimer = 0.0f;
        return;
    }

    if (activeCount >= maxConcurrent) return;
    if (m_spawnTimer <= spawnRate) return;
    m_spawnTimer = 0.0f;

    // 构建生成池
    std::vector<EnemyType> pool;
    for (int i = 0; i < 4; i++) pool.push_back(EnemyType::SMALL);
    for (int i = 0; i < 3; i++) pool.push_back(EnemyType::NORMAL);
    if (m_wave >= 3) pool.push_back(EnemyType::LARGE);
    if (m_wave >= 5) { pool.push_back(EnemyType::LARGE); }

    // 特殊敌人检查
    int specCap = 1;
    if (m_wave >= 8) specCap = 1;
    if (m_wave >= 12) specCap = 2;
    if (m_wave >= 15) specCap = 3;
    if (m_wave >= 18) specCap = 4;

    int specNum = 0;
    for (auto* e : m_enemies) {
        if (!e->IsAlive()) continue;
        EnemyType et = e->GetType();
        if (et == EnemyType::FAST || et == EnemyType::SHIELD ||
            et == EnemyType::SPLITTER || et == EnemyType::SNIPER) specNum++;
    }

    if (m_wave >= 4 && specNum < specCap) {
        bool hasFast = false;
        for (auto* e : m_enemies) if (e->IsAlive() && e->GetType() == EnemyType::FAST) hasFast = true;
        if (!hasFast) pool.push_back(EnemyType::FAST);
    }
    if (m_wave >= 6 && specNum < specCap) {
        bool hasShield = false;
        for (auto* e : m_enemies) if (e->IsAlive() && e->GetType() == EnemyType::SHIELD) hasShield = true;
        if (!hasShield) pool.push_back(EnemyType::SHIELD);
    }
    if (m_wave >= 8 && specNum < specCap) {
        bool hasSplitter = false;
        for (auto* e : m_enemies) if (e->IsAlive() && e->GetType() == EnemyType::SPLITTER) hasSplitter = true;
        if (!hasSplitter) pool.push_back(EnemyType::SPLITTER);
    }
    if (m_wave >= 10 && specNum < specCap) {
        bool hasSniper = false;
        for (auto* e : m_enemies) if (e->IsAlive() && e->GetType() == EnemyType::SNIPER) hasSniper = true;
        if (!hasSniper) pool.push_back(EnemyType::SNIPER);
    }

    EnemyType type = pool[RandomInt(0, (int)pool.size() - 1)];
    float x = 50.0f + RandomFloat(0.0f, (float)(WINDOW_WIDTH - 100));
    m_enemies.push_back(new Enemy(x, -30.0f, type));
}

// ============================================================
// EnemyShoot / BossShoot
// ============================================================
void GameManager::EnemyShoot(Enemy* enemy) {
    Vector2 pos = enemy->GetShootPos();
    Player* p = m_player;
    ShootPattern pattern = enemy->GetPattern();

    switch (pattern) {
        case ShootPattern::SINGLE: {
            Vector2 dir = (Vector2(p->GetX(), p->GetY()) - pos).Normalized();
            float spd = 200.0f;
            m_enemyBullets.push_back(new EnemyBullet(
                pos.x, pos.y, dir.x * spd, dir.y * spd,
                RGB(255, 68, 68), 3.0f));
            break;
        }
        case ShootPattern::SPREAD: {
            Vector2 dir = (Vector2(p->GetX(), p->GetY()) - pos);
            float baseAngle = std::atan2(dir.y, dir.x);
            for (int i = -2; i <= 2; i++) {
                float angle = baseAngle + i * 0.2f;
                m_enemyBullets.push_back(new EnemyBullet(
                    pos.x, pos.y,
                    std::cos(angle) * 180.0f, std::sin(angle) * 180.0f,
                    RGB(255, 128, 0), 3.0f));
            }
            break;
        }
        case ShootPattern::SPIRAL: {
            enemy->SetSpiralAngle(enemy->GetSpiralAngle() + 0.3f);
            float sa = enemy->GetSpiralAngle();
            for (int i = 0; i < 3; i++) {
                float angle = sa + (PI * 2.0f / 3.0f) * i;
                m_enemyBullets.push_back(new EnemyBullet(
                    pos.x, pos.y,
                    std::cos(angle) * 150.0f, std::sin(angle) * 150.0f,
                    RGB(255, 34, 34), 3.5f));
            }
            break;
        }
        case ShootPattern::LASER: {
            enemy->SetPhase(enemy->GetPhase() + m_deltaTime);
            float phase = enemy->GetPhase();
            if ((int)(phase / 3.0f) % 2 == 0) {
                enemy->SetSpiralAngle(enemy->GetSpiralAngle() + 0.05f);
                float sa = enemy->GetSpiralAngle();
                for (int i = 0; i < 16; i++) {
                    float angle = sa + (PI * 2.0f / 16.0f) * i;
                    m_enemyBullets.push_back(new EnemyBullet(
                        pos.x, pos.y,
                        std::cos(angle) * 120.0f, std::sin(angle) * 120.0f,
                        RGB(255, 255, 0), 4.0f));
                }
            } else {
                Vector2 dir = (Vector2(p->GetX(), p->GetY()) - pos);
                float aim = std::atan2(dir.y, dir.x);
                for (int i = -4; i <= 4; i++) {
                    float angle = aim + i * 0.12f;
                    m_enemyBullets.push_back(new EnemyBullet(
                        pos.x, pos.y,
                        std::cos(angle) * 200.0f, std::sin(angle) * 200.0f,
                        RGB(255, 0, 255), 3.5f));
                }
            }
            break;
        }
        case ShootPattern::SNIPER: {
            if (!enemy->HasWarningLine()) {
                enemy->SetWarningLine(true);
                enemy->SetWarnTimer(0.5f);
                Vector2 dir = (Vector2(p->GetX(), p->GetY()) - pos);
                enemy->SetSniperAngle(std::atan2(dir.y, dir.x));
            } else {
                enemy->SetWarnTimer(enemy->GetWarnTimer() - m_deltaTime);
                if (enemy->GetWarnTimer() <= 0.0f) {
                    float a = enemy->GetSniperAngle();
                    m_enemyBullets.push_back(new EnemyBullet(
                        pos.x, pos.y,
                        std::cos(a) * 450.0f, std::sin(a) * 450.0f,
                        RGB(255, 0, 136), 5.0f));
                    enemy->SetWarningLine(false);
                    enemy->SetWarnTimer(0.0f);
                }
            }
            break;
        }
    }
}

// ============================================================
// BossShoot - BOSS攻击 (5种模式)
// ============================================================
void GameManager::BossShoot(Boss* boss) {
    if (!boss || !boss->IsAlive()) return;

    float px = m_player->GetX(), py = m_player->GetY();
    Vector2 pos = boss->GetShootPos();
    float bPhase = (float)boss->GetBossPhase();

    // ringBurst: 环形爆裂
    if (boss->HasRingBurst() && boss->GetRingBurstTimer() <= 0.0f) {
        float mult = (bPhase >= 2.0f) ? boss->GetPhase3SpeedMult() : 1.0f;
        boss->SetRingBurstTimer(4.5f * mult);
        int count = (bPhase >= 2.0f) ? 24 : 18;
        for (int i = 0; i < count; i++) {
            float a = (PI * 2.0f / count) * (float)i;
            m_enemyBullets.push_back(new EnemyBullet(
                pos.x, pos.y, std::cos(a) * 90.0f, std::sin(a) * 90.0f,
                RGB(255, 68, 68), 3.5f));
        }
        if (bPhase >= 2.0f) {
            for (int i = 0; i < 12; i++) {
                float a = (PI * 2.0f / 12.0f) * (float)i + 0.2f;
                m_enemyBullets.push_back(new EnemyBullet(
                    pos.x, pos.y, std::cos(a) * 60.0f, std::sin(a) * 60.0f,
                    RGB(255, 128, 0), 3.0f));
            }
        }
        m_particles->EmitRing(pos.x, pos.y, count, RGB(255, 68, 68), 0.3f, 100.0f, 3.0f);
    }

    // aimedFan: 瞄准扇形
    if (boss->HasAimedFan() && boss->GetAimedFanTimer() <= 0.0f) {
        float mult = (bPhase >= 2.0f) ? boss->GetPhase3SpeedMult() : 1.0f;
        boss->SetAimedFanTimer(2.8f * mult);
        float aimAngle = std::atan2(py - pos.y, px - pos.x);
        int count = (bPhase >= 2.0f) ? 12 : 8;
        for (int i = -count/2; i <= count/2; i++) {
            float a = aimAngle + (float)i * 0.1f;
            m_enemyBullets.push_back(new EnemyBullet(
                pos.x, pos.y, std::cos(a) * 220.0f, std::sin(a) * 220.0f,
                RGB(255, 128, 0), 3.5f));
        }
        m_particles->EmitDirected(pos.x, pos.y, 8, RGB(255, 128, 0), aimAngle, 0.2f, 80.0f, 2.0f, 0.8f);
    }

    // warnBeam: 预警激光
    if (boss->HasWarnBeam() && boss->GetWarnBeamTimer() <= 0.0f && !boss->IsWarnBeamActive()) {
        boss->SetWarnBeamTimer(10.0f);
        boss->SetWarnBeamActive(true);
        boss->SetWarnBeamPhase(1);
        boss->SetWarnBeamTimer2(0.6f);
        boss->SetWarnBeamAngle(std::atan2(py - pos.y, px - pos.x));
        // 狙击线在Renderer中绘制
    }

    // 预警激光发射中
    if (boss->IsWarnBeamActive() && boss->GetWarnBeamPhase() == 2) {
        float angle = boss->GetWarnBeamAngle();
        for (int i = 0; i < 8; i++) {
            float offset = ((float)i - 3.5f) * 8.0f;
            float a = angle + (RandomFloat(0.0f, 1.0f) - 0.5f) * 0.15f;
            float spd = 500.0f + RandomFloat(0.0f, 100.0f);
            m_enemyBullets.push_back(new EnemyBullet(
                pos.x, pos.y, std::cos(a) * spd, std::sin(a) * spd,
                RGB(255, 255, 80), 4.0f));
        }
    }

    // bounce: 反弹弹
    if (boss->HasBounce() && boss->GetBounceTimer() <= 0.0f) {
        float mult = (bPhase >= 2.0f) ? boss->GetPhase3SpeedMult() : 1.0f;
        boss->SetBounceTimer(6.0f * mult);
        for (int i = 0; i < (bPhase >= 2.0f ? 6 : 4); i++) {
            float a = RandomFloat(0.0f, PI * 2.0f);
            float spd = 130.0f + RandomFloat(0.0f, 60.0f);
            m_enemyBullets.push_back(new EnemyBullet(
                pos.x, pos.y, std::cos(a) * spd, std::sin(a) * spd,
                RGB(80, 255, 80), 4.0f, BulletBehavior::BOUNCE));
        }
    }

    // laserOrb: 能量球
    if (boss->HasLaserOrb() && boss->GetLaserOrbTimer() <= 0.0f) {
        float mult = (bPhase >= 2.0f) ? boss->GetPhase3SpeedMult() : 1.0f;
        boss->SetLaserOrbTimer(9.0f * mult);
        m_enemyBullets.push_back(new EnemyBullet(
            pos.x, pos.y, RandomFloat(-30.0f, 30.0f), RandomFloat(-40.0f, -20.0f),
            RGB(255, 255, 80), 8.0f, BulletBehavior::LASER_ORB));
    }
}

// ============================================================
// HandleCollisions
// ============================================================
void GameManager::HandleCollisions() {
    // 玩家子弹 vs 敌人
    Collision::CheckPlayerBulletEnemy(m_playerBullets, m_enemies, m_particles, this);

    // 敌人子弹 vs 玩家
    if (m_player->GetInvincible() <= 0.0f) {
        if (Collision::CheckEnemyBulletPlayer(m_enemyBullets, m_player)) {
            m_player->TakeDamage(10);
            if (m_player->GetHP() <= 0) {
                GameOver();
            }
        }
    }

    // BOSS碰撞
    if (m_boss && m_boss->IsAlive()) {
        if (m_player->GetInvincible() <= 0.0f) {
            if (Collision::CheckBossPlayerCollision(m_boss, m_player)) {
                m_player->TakeDamage(20);
                if (m_player->GetHP() <= 0) {
                    Defeat();
                }
            }
        }
    }

    if (m_infiniteBoss && m_infiniteBoss->IsAlive()) {
        if (m_player->GetInvincible() <= 0.0f) {
            if (Collision::CheckBossPlayerCollision(m_infiniteBoss, m_player)) {
                m_player->TakeDamage(20);
                if (m_player->GetHP() <= 0) {
                    GameOver();
                }
            }
        }
    }
}

// ============================================================
// UpdateBullets / UpdateEnemies
// ============================================================
void GameManager::UpdateBullets(float dt) {
    for (int i = (int)m_playerBullets.size() - 1; i >= 0; i--) {
        m_playerBullets[i]->Update(dt);
        if (!m_playerBullets[i]->IsAlive()) {
            delete m_playerBullets[i];
            m_playerBullets[i] = m_playerBullets.back();
            m_playerBullets.pop_back();
        }
    }
    for (int i = (int)m_enemyBullets.size() - 1; i >= 0; i--) {
        m_enemyBullets[i]->Update(dt, WINDOW_WIDTH, WINDOW_HEIGHT);
        // laserOrb子弹生成
        if (m_enemyBullets[i]->GetBehavior() == BulletBehavior::LASER_ORB &&
            m_enemyBullets[i]->GetBurstTimer() <= 0.0f &&
            m_enemyBullets[i]->GetLifeTimer() < m_enemyBullets[i]->GetMaxLife()) {
            m_enemyBullets[i]->SetBurstTimer(0.15f);
            for (int j = 0; j < 5; j++) {
                float a = (PI * 2.0f / 5.0f) * j + m_enemyBullets[i]->GetTime() * 0.5f;
                m_enemyBullets.push_back(new EnemyBullet(
                    m_enemyBullets[i]->GetX(), m_enemyBullets[i]->GetY(),
                    std::cos(a) * 80.0f, std::sin(a) * 80.0f,
                    RGB(255, 128, 0), 2.5f));
            }
        }
        if (!m_enemyBullets[i]->IsAlive()) {
            delete m_enemyBullets[i];
            m_enemyBullets[i] = m_enemyBullets.back();
            m_enemyBullets.pop_back();
        }
    }
}

void GameManager::UpdateEnemies(float dt) {
    for (int i = (int)m_enemies.size() - 1; i >= 0; i--) {
        m_enemies[i]->Update(dt);
        if (!m_enemies[i]->IsAlive()) {
            delete m_enemies[i];
            m_enemies[i] = m_enemies.back();
            m_enemies.pop_back();
        }
    }
}

// ============================================================
// UpdateCommonEffects
// ============================================================
void GameManager::UpdateCommonEffects(float dt) {
    // 回血闪动
    if (m_healFlashTimer > 0.0f) m_healFlashTimer -= dt;

    // 屏幕震动
    if (m_shakeDuration > 0.0f) {
        m_shakeDuration -= dt;
        m_shakeX = (RandomFloat(0.0f, 1.0f) - 0.5f) * m_shakeIntensity * 2.0f;
        m_shakeY = (RandomFloat(0.0f, 1.0f) - 0.5f) * m_shakeIntensity * 2.0f;
    } else {
        m_shakeX = 0.0f;
        m_shakeY = 0.0f;
        m_shakeIntensity = 0.0f;
    }

    // 屏幕闪光
    m_screenFlash = (std::max)(0.0f, m_screenFlash - dt * 2.0f);
}

void GameManager::AddScreenShake(float intensity, float duration) {
    if (intensity > m_shakeIntensity || m_shakeDuration <= 0.0f) {
        m_shakeIntensity = intensity;
        m_shakeDuration = duration;
    }
}

void GameManager::AddScreenFlash(float alpha) {
    m_screenFlash = (std::max)(m_screenFlash, alpha);
}

// ============================================================
// 游戏流程控制
// ============================================================
void GameManager::StartGame() {
    // 清理
    for (auto* e : m_enemies) delete e;
    m_enemies.clear();
    for (auto* b : m_playerBullets) delete b;
    m_playerBullets.clear();
    for (auto* b : m_enemyBullets) delete b;
    m_enemyBullets.clear();
    m_particles->Clear();
    if (m_boss) { delete m_boss; m_boss = nullptr; }
    if (m_infiniteBoss) { delete m_infiniteBoss; m_infiniteBoss = nullptr; }

    // 重置状态
    m_bossMode = false;
    m_totalKills = 0;
    m_currentStage = 0;
    m_score = 0;
    m_wave = 1;
    m_waveTimer = 0.0f;
    m_spawnTimer = 0.0f;
    m_comboCount = 0;
    m_maxCombo = 0;
    m_comboTimer = 0.0f;
    m_transTimer = 0.0f;
    m_transStage = -1;
    m_laserActive = false;
    m_laserTickTimer = 0.0f;
    m_slowMotion = 1.0f;
    m_ultimateActive = false;
    m_ultimateTimer = 0.0f;
    m_shakeX = 0.0f; m_shakeY = 0.0f;
    m_shakeIntensity = 0.0f; m_shakeDuration = 0.0f;
    m_screenFlash = 0.0f;
    m_healFlashTimer = 0.0f;
    m_killFlashTimer = 0.0f;
    m_nextInfiniteBossKill = NEXT_INFINITE_BOSS;
    m_infiniteBossCount = 0;

    // 重建玩家
    if (m_player) delete m_player;
    m_player = new Player(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.75f);

    // 入场动画
    m_introPhase = 1;
    m_introTimer = 0.0f;
    m_gameState = GameState::INTRO;  // ★ 切换到INTRO状态

    Input::GetInstance().ResetOnEnterGame();
    Audio::GetInstance().PlayUIClick();
}

void GameManager::StartBossGame() {
    // 清理
    for (auto* e : m_enemies) delete e;
    m_enemies.clear();
    for (auto* b : m_playerBullets) delete b;
    m_playerBullets.clear();
    for (auto* b : m_enemyBullets) delete b;
    m_enemyBullets.clear();
    m_particles->Clear();
    if (m_boss) { delete m_boss; m_boss = nullptr; }
    if (m_infiniteBoss) { delete m_infiniteBoss; m_infiniteBoss = nullptr; }

    // 重置
    m_bossMode = true;
    m_totalKills = 0;
    m_currentStage = 0;
    m_score = 0;
    m_wave = 1;
    m_comboCount = 0;
    m_maxCombo = 0;
    m_comboTimer = 0.0f;
    m_transTimer = 0.0f;
    m_transStage = -1;
    m_laserActive = false;
    m_laserTickTimer = 0.0f;
    m_slowMotion = 1.0f;
    m_ultimateActive = false;
    m_ultimateTimer = 0.0f;
    m_bossGameTimer = 0.0f;
    m_bossDamageDealt = 0;
    m_bossFinishTime = 0.0f;
    m_bossPhase = 0;
    m_shakeX = 0.0f; m_shakeY = 0.0f;
    m_shakeIntensity = 0.0f; m_shakeDuration = 0.0f;
    m_screenFlash = 0.0f;

    if (m_player) delete m_player;
    m_player = new Player(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT * 0.75f);

    m_boss = new Boss(WINDOW_WIDTH / 2.0f, 80.0f, BOSS_HP, false);

    m_introPhase = 1;
    m_introTimer = 0.0f;
    m_gameState = GameState::INTRO;  // ★ 切换到INTRO状态

    Input::GetInstance().ResetOnEnterGame();
    Audio::GetInstance().PlayUIClick();
}

void GameManager::RestartGame() {
    StartGame();
}

void GameManager::RestartBossGame() {
    StartBossGame();
}

void GameManager::ShowMainMenu() {
    m_gameState = GameState::MAIN_MENU;
    m_introPhase = 0;
    m_introTimer = 0.0f;
    Input::GetInstance().ResetInputState(); // ★ 防穿透

    // 清理游戏实体
    for (auto* e : m_enemies) delete e;
    m_enemies.clear();
    for (auto* b : m_playerBullets) delete b;
    m_playerBullets.clear();
    for (auto* b : m_enemyBullets) delete b;
    m_enemyBullets.clear();
    m_particles->Clear();
    if (m_boss) { delete m_boss; m_boss = nullptr; }
    if (m_infiniteBoss) { delete m_infiniteBoss; m_infiniteBoss = nullptr; }

    Renderer::GetInstance().ShowBossHPBar(false);
}

void GameManager::GameOver() {
    m_gameState = GameState::GAME_OVER;
    if (m_score > m_highScore) m_highScore = m_score;
    Renderer::GetInstance().ShowBossHPBar(false);
    Input::GetInstance().ResetInputState(); // ★ 防穿透
}

void GameManager::Victory() {
    m_gameState = GameState::VICTORY;
    if (m_score > m_highScore) m_highScore = m_score;
    Renderer::GetInstance().ShowBossHPBar(false);
    Input::GetInstance().ResetInputState(); // ★ 防穿透
}

void GameManager::Defeat() {
    m_gameState = GameState::DEFEAT;
    Renderer::GetInstance().ShowBossHPBar(false);
    Input::GetInstance().ResetInputState(); // ★ 防穿透
}
