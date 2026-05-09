#pragma once

#include "Common.h"
#include <windows.h>

struct BuffData;

class SceneManager;
class UIManager;
class CollisionManager;
class EntityManager;
class Player;
class BuffManager;

// ============================================================
// GameManager - 全局单例，控制游戏主循环和状态切换
// ============================================================
class GameManager {
public:
    static GameManager& GetInstance();

    // 禁止拷贝
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    // 核心生命周期
    void Init();
    void Run();                    // 自驱动主循环（内部 while）
    void ProcessFrame(float dt);   // 单帧处理（由外部主循环驱动）
    void Shutdown();

    // 状态切换
    void SetState(GameState newState);
    GameState GetState() const { return m_currentState; }
    GameState GetPrevState() const { return m_previousState; }
    bool IsState(GameState state) const { return m_currentState == state; }

    // 关卡控制
    void StartNewGame(Profession prof);
    void NextLevel();
    void RestartLevel();
    void GameOver(bool victory);

    // 当前信息
    int  GetCurrentLevel() const { return m_currentLevel; }
    BiomeType GetCurrentBiome() const;
    int  GetGold() const { return m_gold; }
    void AddGold(int amount);
    bool SpendGold(int amount);

    // 作弊模式
    bool IsCheatActive() const { return m_cheatActive; }
    void SetCheatActive(bool v) { m_cheatActive = v; }

    // 子管理器访问
    SceneManager&    GetSceneManager()    { return *m_pSceneManager; }
    UIManager&       GetUIManager()       { return *m_pUIManager; }
    CollisionManager& GetCollisionManager() { return *m_pCollisionManager; }
    EntityManager&   GetEntityManager()   { return *m_pEntityManager; }
    BuffManager&     GetBuffManager()     { return *m_pBuffManager; }
    Player*          GetPlayer()          { return m_pPlayer; }
    void             SetPlayer(Player* p) { m_pPlayer = p; }

    // 帧率相关
    float GetDeltaTime() const { return m_deltaTime; }
    float GetStateDelayTimer() const { return m_stateDelayTimer; }
    int   GetFPS() const { return m_fps; }
    float GetElapsedTime() const { return m_elapsedTime; }

    // 窗口句柄（EasyX 需要）
    HWND  GetHWND() const { return m_hWnd; }

private:
    GameManager();
    ~GameManager();

    void ProcessInput();
    void Update();
    void Render();
    void CalculateFPS();

    GameState  m_currentState;
    GameState  m_previousState;
    int        m_currentLevel;
    int        m_gold;
    bool       m_running;

    float      m_deltaTime;
    float      m_elapsedTime;
    int        m_fps;
    int        m_frameCount;
    float      m_fpsTimer;

    HWND       m_hWnd;

    // 子管理器（前向声明 + unique_ptr）
    std::unique_ptr<SceneManager>     m_pSceneManager;
    std::unique_ptr<UIManager>        m_pUIManager;
    std::unique_ptr<CollisionManager> m_pCollisionManager;
    std::unique_ptr<EntityManager>    m_pEntityManager;
    std::unique_ptr<BuffManager>      m_pBuffManager;

    Player*    m_pPlayer;

    // 天赋选择缓存（进入 TALENT_SELECT 时随机一次，避免每帧重随）
    std::vector<BuffData*> m_cachedTalentRolls;
    bool                   m_talentsRolled;

    // 鼠标去抖：状态切换后抑制首次点击，等待鼠标释放
    bool                   m_mousePrev;

    // 状态切换延迟：防止点击穿透
    float                  m_stateDelayTimer;

    // 作弊模式
    bool                   m_cheatActive;
};
