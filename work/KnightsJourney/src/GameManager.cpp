// ============================================================
// GameManager.cpp - 全局单例，游戏主循环与状态管理
// ============================================================

#include "../include/Core/GameManager.h"

// 子管理器
#include "../include/System/SceneManager.h"
#include "../include/System/MapGenerator.h"
#include "../include/System/CollisionManager.h"
#include "../include/System/EntityManager.h"
#include "../include/System/BuffManager.h"

// 实体
#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Entity/Boss.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/DropItem.h"
#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Chest.h"
#include "../include/Entity/Door.h"

// UI
#include "../include/UI/UIManager.h"

// EasyX
#include <graphics.h>
#include <windows.h>

// ============================================================
// 单例实现
// ============================================================
GameManager& GameManager::GetInstance() {
    static GameManager instance;
    return instance;
}

GameManager::GameManager()
    : m_currentState(GameState::MAIN_MENU)
    , m_previousState(GameState::MAIN_MENU)
    , m_currentLevel(0)
    , m_gold(0)
    , m_running(false)
    , m_deltaTime(0.0f)
    , m_elapsedTime(0.0f)
    , m_fps(0)
    , m_frameCount(0)
    , m_fpsTimer(0.0f)
    , m_hWnd(nullptr)
    , m_pPlayer(nullptr)
    , m_talentsRolled(false)
    , m_mousePrev(false)
    , m_stateDelayTimer(0.0f)
    , m_cheatActive(false) {
}

GameManager::~GameManager() {
}

// ============================================================
// 初始化
// ============================================================
void GameManager::Init() {
    m_hWnd = GetHWnd();

    // 创建子管理器
    m_pEntityManager    = std::unique_ptr<EntityManager>(new EntityManager());
    m_pCollisionManager = std::unique_ptr<CollisionManager>(new CollisionManager());
    m_pSceneManager     = std::unique_ptr<SceneManager>(new SceneManager());
    m_pBuffManager      = std::unique_ptr<BuffManager>(new BuffManager());
    m_pUIManager        = std::unique_ptr<UIManager>(new UIManager());

    // 初始化子管理器
    m_pSceneManager->Init(m_pEntityManager.get());
    m_pUIManager->Init(m_hWnd);
    m_pBuffManager->Init();

    // 设置碰撞回调
    m_pCollisionManager->RegisterBulletHitCallback(
        [this](GameObject* bullet, GameObject* target) {
            // 子弹命中逻辑在 CollisionManager::ResolveBulletEnemyHit 中处理
        }
    );

    // 设置初始状态
    SetState(GameState::MAIN_MENU);
    m_running = true;

    // 输出调试信息到控制台
    printf("[GameManager] Initialized.\n");
    printf("[GameManager] Window: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    printf("[GameManager] Entering Main Menu...\n");
}

// ============================================================
// 清理
// ============================================================
void GameManager::Shutdown() {
    m_running = false;
    m_pEntityManager->ClearAll();
    m_pPlayer = nullptr;
    printf("[GameManager] Shutdown complete.\n");
}

// ============================================================
// 完整主循环（独立驱动，不含外部帧率控制）
// ============================================================
void GameManager::Run() {
    if (!m_running) {
        printf("[GameManager] Run() called but not running!\n");
        return;
    }

    // 简易帧计时
    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    while (m_running) {
        QueryPerformanceCounter(&now);
        m_deltaTime = static_cast<float>(now.QuadPart - last.QuadPart) / freq.QuadPart;
        last = now;

        // 帧率尖刺保护
        if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;
        m_elapsedTime += m_deltaTime;

        ProcessFrame(m_deltaTime);
    }
}

// ============================================================
// 单帧处理（由外部主循环调用）
// ============================================================
void GameManager::ProcessFrame(float dt) {
    m_deltaTime = dt;
    m_elapsedTime += dt;

    ProcessInput();
    Update();
    Render();
}

// ============================================================
// 输入处理
// ============================================================
void GameManager::ProcessInput() {
    // 公用鼠标状态（状态切换时 m_mousePrev 被置为 true，抑制穿透点击）
    bool mouseNow = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
    bool mouseClick = mouseNow && !m_mousePrev && m_stateDelayTimer <= 0.0f;

    switch (m_currentState) {
        case GameState::MAIN_MENU: {
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);

            int result = m_pUIManager->HandleMainMenuInput(mx, my, mouseClick);
            if (result == 0) {
                SetState(GameState::CHARACTER_SELECT);
            } else if (result == 1) {
                m_running = false;
                PostQuitMessage(0);
            }
            break;
        }

        case GameState::CHARACTER_SELECT: {
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);

            int result = m_pUIManager->HandleCharacterSelectInput(mx, my, mouseClick);
            if (result == 0) {
                StartNewGame(Profession::KNIGHT);
            } else if (result == 1) {
                StartNewGame(Profession::ROGUE);
            } else if (result == 2) {
                StartNewGame(Profession::MAGE);
            }
            break;
        }

        case GameState::PLAYING: {
            if (!m_pPlayer) break;

            // 暂停键（防按键穿透）
            static bool escapePrev = false;
            bool escapeNow = GetAsyncKeyState(VK_ESCAPE) & 0x8000;
            if (escapeNow && !escapePrev && m_stateDelayTimer <= 0.0f) {
                SetState(GameState::PAUSED);
            }
            escapePrev = escapeNow;

            // 在已清理的 Boss 房间按 Enter 进入下一关
            const RoomData* currentRoom = m_pSceneManager->GetCurrentRoom();
            if (currentRoom && currentRoom->type == RoomType::BOSS && currentRoom->isCleared) {
                static bool enterPrev = false;
                bool enterNow = GetAsyncKeyState(VK_RETURN) & 0x8000;
                if (enterNow && !enterPrev) {
                    if (m_currentLevel >= 3) {
                        GameOver(true);
                    } else {
                        NextLevel();
                    }
                }
                enterPrev = enterNow;
            }

            // 作弊键已禁用
            // static bool cheatPrev = false;
            // bool cheatNow = GetAsyncKeyState('E') & 0x8000;
            // if (cheatNow && !cheatPrev) { ... }

            // 玩家键盘输入
            m_pPlayer->ProcessKeyboardInput(m_deltaTime);
            // 玩家鼠标输入
            m_pPlayer->ProcessMouseInput();

            break;
        }

        case GameState::PAUSED: {
            static bool escapePrev2 = false;
            bool escapeNow = GetAsyncKeyState(VK_ESCAPE) & 0x8000;
            if (escapeNow && !escapePrev2 && m_stateDelayTimer <= 0.0f) {
                SetState(GameState::PLAYING);
            }
            escapePrev2 = escapeNow;

            // 暂停菜单鼠标点击
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);
            if (mouseClick) {
                int result = m_pUIManager->HandlePauseMenuInput(mx, my, true);
                if (result == 0) {  // 继续
                    SetState(GameState::PLAYING);
                } else if (result == 1) {  // 返回主菜单
                    m_pEntityManager->ClearAll();
                    m_pPlayer = nullptr;
                    SetState(GameState::MAIN_MENU);
                }
            }
            break;
        }

        case GameState::SHOP: {
            if (!m_pPlayer) break;
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);
            if (mouseClick) {
                int result = m_pUIManager->HandleShopInput(mx, my, true);
                if (result >= 0 && result < 3) {
                    auto& items = m_pUIManager->GetShopItemsMutable();
                    ShopItemData& item = items[result];
                    if (!item.isSold) {
                        int price = (int)(item.price * m_pBuffManager->GetShopDiscount());
                        if (SpendGold(price)) {
                            item.isSold = true;
                            // 同步更新缓存，保证下次进商店状态一致
                            auto& cached = m_pUIManager->GetCachedShopItems();
                            if (result < (int)cached.size()) {
                                cached[result].isSold = true;
                            }
                            if (item.itemType == ShopItemType::WEAPON) {
                                // 检查是否是天赋（通过名字匹配）
                                bool isTalent = false;
                                for (auto& def : m_pBuffManager->GetAllBuffDefs()) {
                                    if (def.name == item.name) {
                                        m_pBuffManager->ApplyBuff(def.type, m_pPlayer);
                                        isTalent = true;
                                        break;
                                    }
                                }
                                if (!isTalent) {
                                    if (!m_pPlayer->EquipWeapon(item.weaponType)) {
                                        SetState(GameState::WEAPON_SELECT);
                                    }
                                }
                            }
                            printf("[Shop] Purchased: %s\n", item.name.c_str());
                        }
                    }
                }
            }
            // B 或 ESC 离开商店
            static bool shopLeavePrev = false;
            bool shopLeaveNow = (GetAsyncKeyState('B') & 0x8000) ||
                               (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
            if (shopLeaveNow && !shopLeavePrev) {
                m_stateDelayTimer = 0.2f;  // 防 ESC 穿透到暂停
                SetState(GameState::PLAYING);
            }
            shopLeavePrev = shopLeaveNow;
            break;
        }

        case GameState::TALENT_SELECT: {
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);
            if (mouseClick) {
                int result = m_pUIManager->HandleTalentSelectionInput(mx, my, true);
                if (result >= 0 && result <= 2) {
                    // 通过 UIManager 获取选中的天赋
                    const auto& options = m_pUIManager->GetTalentOptions();
                    if (result < static_cast<int>(options.size())) {
                        BuffType selected = options[result].buffType;
                        m_pBuffManager->ApplyBuff(selected, m_pPlayer);
                    }
                    // 回到游戏，玩家自行选择何时前往下一关
                    SetState(GameState::PLAYING);
                }
            }
            break;
        }

        case GameState::WEAPON_SELECT: {
            if (!m_pPlayer || !m_pPlayer->HasPendingWeapon()) {
                SetState(GameState::PLAYING);
                break;
            }
            // ESC 放弃拾取
            static bool wsEscPrev = false;
            bool wsEscNow = GetAsyncKeyState(VK_ESCAPE) & 0x8000;
            if (wsEscNow && !wsEscPrev) {
                m_pPlayer->ClearPendingWeapon();
                m_stateDelayTimer = 0.2f;  // 防 ESC 穿透到暂停
                SetState(GameState::PLAYING);
                break;
            }
            wsEscPrev = wsEscNow;

            int mx, my;
            m_pUIManager->GetMousePos(mx, my);
            if (mouseClick) {
                int result = m_pUIManager->HandleWeaponSelectInput(mx, my, m_pPlayer, true);
                if (result == 0 || result == 1) {
                    m_pPlayer->ReplaceWeapon(result, m_pPlayer->TakePendingWeapon());
                    m_stateDelayTimer = 0.2f;
                    SetState(GameState::PLAYING);
                } else if (result == -1) {
                    m_pPlayer->ClearPendingWeapon();
                    m_stateDelayTimer = 0.2f;
                    SetState(GameState::PLAYING);
                }
            }
            break;
        }

        case GameState::GAME_OVER:
        case GameState::VICTORY: {
            int mx, my;
            m_pUIManager->GetMousePos(mx, my);
            if (m_stateDelayTimer > 0.0f) break;
            // 按键防穿透（上升沿检测）
            static bool enterPrev3 = false;
            bool enterNow = GetAsyncKeyState(VK_RETURN) & 0x8000;
            bool spaceNow = GetAsyncKeyState(VK_SPACE) & 0x8000;
            bool triggered = false;
            if (enterNow && !enterPrev3) triggered = true;
            if (spaceNow || mouseClick) triggered = true;
            enterPrev3 = enterNow;
            if (triggered) {
                m_pEntityManager->ClearAll();
                m_pPlayer = nullptr;
                m_pBuffManager->Reset();
                SetState(GameState::MAIN_MENU);
            }
            break;
        }

        default:
            break;
    }

    // 每帧更新鼠标状态（用于下一帧的边缘检测）
    m_mousePrev = mouseNow;
}

// ============================================================
// 逻辑更新
// ============================================================
void GameManager::Update() {
    // 状态延迟计时器
    if (m_stateDelayTimer > 0.0f) {
        m_stateDelayTimer -= m_deltaTime;
    }

    switch (m_currentState) {
        case GameState::MAIN_MENU:
        case GameState::CHARACTER_SELECT:
        case GameState::GAME_OVER:
        case GameState::VICTORY:
            break;

        case GameState::PLAYING: {
            // 1. 更新实体管理器
            m_pEntityManager->UpdateAll(m_deltaTime);

            // 2. 更新场景（波次调度、房间状态检查）
            m_pSceneManager->Update(m_deltaTime, *m_pEntityManager);

            // 3. 碰撞检测
            m_pCollisionManager->CheckAllCollisions(*m_pEntityManager);

            // 4. 清理死亡实体
            m_pEntityManager->CleanupDeleted();

            // 5. 检查玩家死亡
            if (m_pPlayer && m_pPlayer->IsDead()) {
                // 检查复活十字章
                if (m_pBuffManager->HasReviveCharm() && m_pPlayer->HasReviveCharm()) {
                    m_pPlayer->OnRevive();
                    m_pBuffManager->ConsumeBuff(BuffType::REVIVE_CHARM, m_pPlayer);
                } else {
                    GameOver(false);
                }
            }

            // 6. 检查房间清空 -> 开门
            if (m_pSceneManager->IsRoomCleared()) {
                m_pSceneManager->OpenAllDoors(*m_pEntityManager);
            }

            break;
        }

        case GameState::PAUSED:
        case GameState::SHOP:
        case GameState::REWARD:
        case GameState::TALENT_SELECT:
        case GameState::WEAPON_SELECT:
        case GameState::TRANSITION:
            break;
    }
}

// ============================================================
// 渲染
// ============================================================
void GameManager::Render() {
    // 清屏
    cleardevice();

    // 绘制深色背景
    setfillcolor(RGB(26, 26, 46));
    solidrectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);

    switch (m_currentState) {
        case GameState::MAIN_MENU: {
            m_pUIManager->RenderMainMenu();
            break;
        }

        case GameState::CHARACTER_SELECT: {
            m_pUIManager->RenderCharacterSelect();
            break;
        }

        case GameState::PLAYING: {
            // 绘制房间背景（根据 Biome 颜色）
            BiomeType biome = m_pSceneManager->GetCurrentBiome();
            COLORREF roomBg;
            switch (biome) {
                case BiomeType::FOREST:   roomBg = RGB(34, 68, 34);  break;  // 森林绿
                case BiomeType::ICE_DUNGEON: roomBg = RGB(40, 60, 80); break; // 冰原蓝
                case BiomeType::VOLCANO:  roomBg = RGB(60, 30, 20);  break;  // 熔岩暗红
                default:                  roomBg = RGB(30, 30, 40);  break;
            }
            // 房间区域
            setfillcolor(roomBg);
            solidrectangle(0, 0, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);

            // 绘制所有实体
            m_pEntityManager->RenderAll();

            // 绘制 HUD（血条、蓝条、小地图、武器栏）
            if (m_pPlayer) {
                const RoomData* currentRoom = m_pSceneManager->GetCurrentRoom();
                m_pUIManager->RenderHUD(m_pPlayer, *m_pBuffManager,
                                        m_currentLevel, biome, m_gold,
                                        currentRoom);
            }
            break;
        }

        case GameState::PAUSED: {
            // 绘制游戏画面（冻结）
            // 先重绘实体，再覆盖暂停遮罩
            BiomeType biome = m_pSceneManager->GetCurrentBiome();
            COLORREF roomBg = RGB(30, 30, 40);
            if (biome == BiomeType::FOREST) roomBg = RGB(34, 68, 34);
            else if (biome == BiomeType::ICE_DUNGEON) roomBg = RGB(40, 60, 80);
            else if (biome == BiomeType::VOLCANO) roomBg = RGB(60, 30, 20);
            setfillcolor(roomBg);
            solidrectangle(0, 0, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);
            m_pEntityManager->RenderAll();

            // 暗色遮罩（EasyX 不支持 alpha 通道，纯黑色半透明效果用图案填充模拟）
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);
            setbkmode(TRANSPARENT);

            m_pUIManager->RenderPauseMenu();
            break;
        }

        case GameState::SHOP: {
            // 绘制商店背景 + 商品
            m_pUIManager->RenderShop(m_pUIManager->GetShopItemsMutable(),
                                     m_gold,
                                     m_pBuffManager->GetShopDiscount());
            break;
        }

        case GameState::REWARD: {
            m_pUIManager->RenderRewardRoom();
            break;
        }

        case GameState::WEAPON_SELECT: {
            // 绘制游戏画面作背景
            BiomeType biome = m_pSceneManager->GetCurrentBiome();
            COLORREF roomBg = RGB(30, 30, 40);
            if (biome == BiomeType::FOREST) roomBg = RGB(34, 68, 34);
            else if (biome == BiomeType::ICE_DUNGEON) roomBg = RGB(40, 60, 80);
            else if (biome == BiomeType::VOLCANO) roomBg = RGB(60, 30, 20);
            setfillcolor(roomBg);
            solidrectangle(0, 0, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);
            m_pEntityManager->RenderAll();
            // 暗色遮罩
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);
            m_pUIManager->RenderWeaponSelection(m_pPlayer);
            break;
        }

        case GameState::TALENT_SELECT: {
            // 只在首次进入天赋选择时随机，避免每帧重随
            if (!m_talentsRolled) {
                m_cachedTalentRolls = m_pBuffManager->RollRandomBuffs(3);
                m_talentsRolled = true;

                // 将天赋数据传递给 UIManager 构建 UI 选项
                std::vector<TalentOption> options;
                for (size_t i = 0; i < m_cachedTalentRolls.size(); ++i) {
                    TalentOption opt;
                    opt.buffType    = m_cachedTalentRolls[i]->type;
                    opt.name        = m_cachedTalentRolls[i]->name;
                    opt.description = m_cachedTalentRolls[i]->description;
                    opt.isSelected  = false;
                    // 布局：三个天赋选项水平排列
                    float panelW = 250.0f;
                    float panelH = 300.0f;
                    float startX = WINDOW_WIDTH / 2.0f - (panelW * 1.5f + 20.0f) + (panelW + 20.0f) * i;
                    float startY = WINDOW_HEIGHT / 2.0f - panelH / 2.0f;
                    opt.bounds = AABB(startX + panelW / 2.0f, startY + panelH / 2.0f,
                                      panelW / 2.0f, panelH / 2.0f);
                    options.push_back(opt);
                }
                m_pUIManager->SetTalentOptions(options);
            }
            m_pUIManager->RenderTalentSelection(m_cachedTalentRolls, *m_pBuffManager);
            break;
        }

        case GameState::GAME_OVER: {
            m_pUIManager->RenderGameOver(false, m_currentLevel, m_gold);
            break;
        }

        case GameState::VICTORY: {
            m_pUIManager->RenderGameOver(true, m_currentLevel, m_gold);
            break;
        }

        case GameState::TRANSITION: {
            // 过渡动画：黑色淡入淡出
            // 简化：直接切黑屏
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);
            break;
        }
    }

    // 刷新双缓冲
    FlushBatchDraw();
}

// ============================================================
// 状态切换
// ============================================================
void GameManager::SetState(GameState newState) {
    if (m_currentState == newState) return;

    m_previousState = m_currentState;
    m_currentState = newState;

    // 进入天赋选择时重置随机缓存
    if (newState == GameState::TALENT_SELECT) {
        m_talentsRolled = false;
        m_cachedTalentRolls.clear();
    }

    // 敏感状态延迟输入，防止点击穿透
    if (newState == GameState::TALENT_SELECT ||
        newState == GameState::GAME_OVER ||
        newState == GameState::VICTORY ||
        newState == GameState::SHOP ||
        newState == GameState::WEAPON_SELECT ||
        newState == GameState::PAUSED) {
        m_stateDelayTimer = 0.25f;
    }

    // 状态切换后抑制鼠标点击，直到用户释放并重新按下
    m_mousePrev = true;

    const char* stateNames[] = {
        "MAIN_MENU", "CHARACTER_SELECT", "PLAYING", "PAUSED",
        "SHOP", "REWARD", "TALENT_SELECT", "WEAPON_SELECT",
        "GAME_OVER", "VICTORY", "TRANSITION"
    };
    int idx = static_cast<int>(newState);
    if (idx >= 0 && idx < 11) {
        printf("[GameManager] State: %s -> %s\n",
               stateNames[static_cast<int>(m_previousState)],
               stateNames[idx]);
    }
}

// ============================================================
// 关卡控制（简化实现）
// ============================================================
void GameManager::StartNewGame(Profession prof) {
    printf("[GameManager] Starting new game...\n");

    m_currentLevel = 1;
    m_gold = 0;
    m_pBuffManager->Reset();

    // 创建玩家
    m_pEntityManager->ClearAll();
    m_pPlayer = m_pEntityManager->SpawnEntity<Player>();
    m_pPlayer->InitProfession(prof);
    m_pPlayer->SetPosition(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);

    // 各职业初始武器不同
    switch (prof) {
        case Profession::KNIGHT: m_pPlayer->EquipWeapon(WeaponType::SHOTGUN);          break;
        case Profession::ROGUE:  m_pPlayer->EquipWeapon(WeaponType::ASSAULT_RIFLE);    break;
        case Profession::MAGE:   m_pPlayer->EquipWeapon(WeaponType::MAGIC_STAFF);      break;
    }

    // 重置商店持久化
    m_pUIManager->SetShopGeneratedThisLevel(false);

    // 加载第一关
    BiomeType startBiome = BiomeType::FOREST;
    m_pSceneManager->LoadLevel(m_currentLevel, startBiome, *m_pEntityManager);

    m_stateDelayTimer = 0.3f;  // 防角色选择点击穿透
    SetState(GameState::PLAYING);
    printf("[GameManager] New game started! Level %d, Biome: FOREST\n", m_currentLevel);
}

void GameManager::NextLevel() {
    m_currentLevel++;
    m_pUIManager->SetShopGeneratedThisLevel(false);  // 重置商店
    BiomeType biome = GetCurrentBiome();
    m_pSceneManager->LoadLevel(m_currentLevel, biome, *m_pEntityManager);

    // 将玩家移到新房间起始位置
    if (m_pPlayer) {
        m_pPlayer->SetPosition(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);
    }

    SetState(GameState::PLAYING);
    printf("[GameManager] Advancing to Level %d\n", m_currentLevel);
}

void GameManager::RestartLevel() {
    BiomeType biome = GetCurrentBiome();
    m_pEntityManager->ClearAll();

    // 重新创建玩家（保留属性和装备）
    // 简化：恢复玩家 HP/MP
    if (m_pPlayer) {
        m_pPlayer->SetHP(m_pPlayer->GetMaxHP());
        m_pPlayer->SetMP(m_pPlayer->GetMaxMP());
        m_pPlayer->SetPosition(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);
        m_pEntityManager->RegisterEntity(
            std::unique_ptr<Player>(m_pPlayer)
        );
    }

    m_pSceneManager->LoadLevel(m_currentLevel, biome, *m_pEntityManager);
    SetState(GameState::PLAYING);
    printf("[GameManager] Level %d restarted.\n", m_currentLevel);
}

void GameManager::GameOver(bool victory) {
    if (victory) {
        SetState(GameState::VICTORY);
        printf("[GameManager] VICTORY! Final Level: %d, Gold: %d\n", m_currentLevel, m_gold);
    } else {
        SetState(GameState::GAME_OVER);
        printf("[GameManager] GAME OVER at Level %d\n", m_currentLevel);
    }
}

// ============================================================
// 资源管理
// ============================================================
BiomeType GameManager::GetCurrentBiome() const {
    // 3 张地图：Lv1=森林, Lv2=冰原, Lv3=火山
    int biomeIndex = ((m_currentLevel - 1) % 3);
    switch (biomeIndex) {
        case 0: return BiomeType::FOREST;
        case 1: return BiomeType::ICE_DUNGEON;
        case 2: return BiomeType::VOLCANO;
        default: return BiomeType::FOREST;
    }
}

void GameManager::AddGold(int amount) {
    if (m_pBuffManager->HasGoldBonus()) {
        amount = static_cast<int>(amount * m_pBuffManager->GetGoldMultiplier());
    }
    m_gold += amount;
}

bool GameManager::SpendGold(int amount) {
    if (m_gold >= amount) {
        m_gold -= amount;
        return true;
    }
    return false;
}

// ============================================================
// FPS 计算
// ============================================================
void GameManager::CalculateFPS() {
    m_frameCount++;
    m_fpsTimer += m_deltaTime;

    if (m_fpsTimer >= 1.0f) {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsTimer -= 1.0f;
    }
}
