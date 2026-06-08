#pragma once

#include <vector>
#include <memory>
#include "Common.h"

class Player;
class Enemy;
class Boss;
class PlayerBullet;
class EnemyBullet;
class Particle;
class ParticleSystem;

// ============================================================
// GameManager - 游戏单例，管理所有状态和实体
// ============================================================
class GameManager {
public:
    static GameManager& GetInstance();

    void Init();
    void Shutdown();
    void ProcessFrame(float dt);

    // 游戏流程控制
    void StartGame();
    void StartBossGame();
    void RestartGame();
    void RestartBossGame();
    void ShowMainMenu();
    void GameOver();
    void Victory();
    void Defeat();

    // 游戏状态
    GameState GetState() const { return m_gameState; }
    void SetState(GameState s) { m_gameState = s; }

    // 游戏时间
    float GetGameTime() const { return m_gameTime; }
    float GetDeltaTime() const { return m_deltaTime; }
    float GetPlayerDt() const { return m_playerDt; }
    float GetSlowMotion() const { return m_slowMotion; }
    void  SetSlowMotion(float s) { m_slowMotion = s; }

    // 模式
    bool IsBossMode() const { return m_bossMode; }

    // 成长系统
    int  GetTotalKills() const { return m_totalKills; }
    void AddKill() { m_totalKills++; }
    int  GetCurrentStage() const { return m_currentStage; }
    void SetCurrentStage(int s) { m_currentStage = s; }
    float GetTransTimer() const { return m_transTimer; }
    void  SetTransTimer(float t) { m_transTimer = t; }
    int  GetTransStage() const { return m_transStage; }
    void SetTransStage(int s) { m_transStage = s; }

    // 激光系统
    bool  IsLaserActive() const { return m_laserActive; }
    void  SetLaserActive(bool a) { m_laserActive = a; }
    float GetLaserTickTimer() const { return m_laserTickTimer; }
    void  SetLaserTickTimer(float t) { m_laserTickTimer = t; }

    // BOSS相关
    Boss* GetBoss() const { return m_boss; }
    void  SetBoss(Boss* b) { m_boss = b; }
    Boss* GetInfiniteBoss() const { return m_infiniteBoss; }
    void  SetInfiniteBoss(Boss* b) { m_infiniteBoss = b; }
    float GetBossGameTimer() const { return m_bossGameTimer; }
    void  SetBossGameTimer(float t) { m_bossGameTimer = t; }
    int   GetBossDamageDealt() const { return m_bossDamageDealt; }
    void  AddBossDamage(int d) { m_bossDamageDealt += d; }
    float GetBossFinishTime() const { return m_bossFinishTime; }
    void  SetBossFinishTime(float t) { m_bossFinishTime = t; }
    int   GetBossPhase() const { return m_bossPhase; }
    void  SetBossPhase(int p) { m_bossPhase = p; }

    // 无限模式Boss
    int   GetNextInfiniteBossKill() const { return m_nextInfiniteBossKill; }
    void  SetNextInfiniteBossKill(int k) { m_nextInfiniteBossKill = k; }
    int   GetInfiniteBossCount() const { return m_infiniteBossCount; }
    void  SetInfiniteBossCount(int c) { m_infiniteBossCount = c; }

    // 大招
    bool  IsUltimateActive() const { return m_ultimateActive; }
    void  SetUltimateActive(bool a) { m_ultimateActive = a; }
    float GetUltimateTimer() const { return m_ultimateTimer; }
    void  SetUltimateTimer(float t) { m_ultimateTimer = t; }

    // 分数/波次/连击
    int   GetScore() const { return m_score; }
    void  AddScore(int s) { m_score += s; }
    int   GetWave() const { return m_wave; }
    void  SetWave(int w) { m_wave = w; }
    int   GetComboCount() const { return m_comboCount; }
    void  SetComboCount(int c) { m_comboCount = c; }
    int   GetMaxCombo() const { return m_maxCombo; }
    void  SetMaxCombo(int c) { m_maxCombo = c; }
    float GetComboTimer() const { return m_comboTimer; }
    void  SetComboTimer(float t) { m_comboTimer = t; }

    // 击杀处理 (公开供Collision调用)
    void KillEnemy(Enemy* enemy);
    void SpawnInfiniteBoss();

    // 屏幕特效
    void AddScreenShake(float intensity, float duration);
    void AddScreenFlash(float alpha);
    float GetScreenShakeX() const { return m_shakeX; }
    float GetScreenShakeY() const { return m_shakeY; }
    float GetScreenShakeIntensity() const { return m_shakeIntensity; }
    float GetScreenFlash() const { return m_screenFlash; }

    // 回血闪动
    float GetHealFlashTimer() const { return m_healFlashTimer; }
    void  SetHealFlashTimer(float t) { m_healFlashTimer = t; }

    // 击杀闪动
    float GetKillFlashTimer() const { return m_killFlashTimer; }
    void  SetKillFlashTimer(float t) { m_killFlashTimer = t; }

    // 滤镜开关
    bool  IsFilterEnabled() const { return m_filterEnabled; }
    void  ToggleFilter() { m_filterEnabled = !m_filterEnabled; }

    // 最高分
    int   GetHighScore() const { return m_highScore; }
    void  SetHighScore(int s) { m_highScore = s; }

    // 入场动画
    int   GetIntroPhase() const { return m_introPhase; }
    void  SetIntroPhase(int p) { m_introPhase = p; }
    float GetIntroTimer() const { return m_introTimer; }
    void  SetIntroTimer(float t) { m_introTimer = t; }

    // 实体容器
    Player* GetPlayer() const { return m_player; }
    std::vector<Enemy*>& GetEnemies() { return m_enemies; }
    std::vector<PlayerBullet*>& GetPlayerBullets() { return m_playerBullets; }
    std::vector<EnemyBullet*>& GetEnemyBullets() { return m_enemyBullets; }
    ParticleSystem* GetParticles() const { return m_particles; }

    // 子弹时间音效状态
    bool GetBulletTimeSoundActive() const { return m_btSoundActive; }
    void SetBulletTimeSoundActive(bool a) { m_btSoundActive = a; }

    // 射击音效计时
    float GetFireSoundTimer() const { return m_fireSoundTimer; }
    void  SetFireSoundTimer(float t) { m_fireSoundTimer = t; }

private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    void UpdateInfiniteMode(float dt, float playerDt);
    void UpdateBossMode(float dt, float playerDt);
    void UpdateCommonEffects(float dt);
    void UpdateBullets(float dt);
    void UpdateEnemies(float dt);
    void HandleCollisions();
    void SpawnEnemies();
    void EnemyShoot(Enemy* enemy);
    void BossShoot(Boss* boss);
    void FireWeapon(float dt);
    void ActivateUltimate();
    void UpdateLaser(float dt);
    void UpdateStage();

    GameState m_gameState = GameState::MAIN_MENU;
    bool m_bossMode = false;

    float m_gameTime = 0.0f;
    float m_deltaTime = 0.0f;
    float m_playerDt = 0.0f;
    float m_slowMotion = 1.0f;
    float m_fireSoundTimer = 0.0f;
    bool  m_btSoundActive = false;

    int   m_totalKills = 0;
    int   m_currentStage = 0;
    float m_transTimer = 0.0f;
    int   m_transStage = -1;

    bool  m_laserActive = false;
    float m_laserTickTimer = 0.0f;

    Boss* m_boss = nullptr;
    Boss* m_infiniteBoss = nullptr;
    float m_bossGameTimer = 0.0f;
    int   m_bossDamageDealt = 0;
    float m_bossFinishTime = 0.0f;
    int   m_bossPhase = 0;

    int   m_nextInfiniteBossKill = NEXT_INFINITE_BOSS;
    int   m_infiniteBossCount = 0;

    bool  m_ultimateActive = false;
    float m_ultimateTimer = 0.0f;

    int   m_score = 0;
    int   m_wave = 1;
    float m_waveTimer = 0.0f;
    float m_spawnTimer = 0.0f;
    int   m_comboCount = 0;
    int   m_maxCombo = 0;
    float m_comboTimer = 0.0f;

    // 屏幕特效
    float m_shakeX = 0.0f, m_shakeY = 0.0f;
    float m_shakeIntensity = 0.0f, m_shakeDuration = 0.0f;
    float m_screenFlash = 0.0f;
    float m_healFlashTimer = 0.0f;
    float m_killFlashTimer = 0.0f;
    bool  m_filterEnabled = true;

    int   m_highScore = 0;
    int   m_introPhase = 0;
    float m_introTimer = 0.0f;

    // 实体
    Player* m_player = nullptr;
    std::vector<Enemy*> m_enemies;
    std::vector<PlayerBullet*> m_playerBullets;
    std::vector<EnemyBullet*> m_enemyBullets;
    ParticleSystem* m_particles = nullptr;
};
