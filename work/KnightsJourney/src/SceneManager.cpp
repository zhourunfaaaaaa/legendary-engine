// ============================================================
// SceneManager.cpp - 场景管理器（房间加载、波次调度、门管理）
// ============================================================

#include "../include/System/SceneManager.h"
#include "../include/System/MapGenerator.h"
#include "../include/System/BuffManager.h"
#include "../include/UI/UIManager.h"
#include "../include/System/EntityManager.h"

#include "../include/Entity/Player.h"
#include "../include/Entity/Enemy.h"
#include "../include/Entity/Boss.h"
#include "../include/Entity/Bullet.h"
#include "../include/Entity/DropItem.h"
#include "../include/Entity/Obstacle.h"
#include "../include/Entity/Chest.h"
#include "../include/Entity/Door.h"
#include "../include/Weapon/WeaponTypes.h"

#include "../include/Core/GameManager.h"

#include <graphics.h>
#include <cstdio>

// ============================================================
SceneManager::SceneManager()
    : m_pEntityMgr(nullptr)
    , m_currentBiome(BiomeType::FOREST)
    , m_currentLevel(1)
    , m_isTransitioning(false)
    , m_transitionTimer(0.0f)
    , m_doorCooldown(0.0f)
    , m_waveCooldown(3.0f) {       // 波间隔 3 秒
    m_map = std::unique_ptr<MapGenerator>(new MapGenerator());
}

SceneManager::~SceneManager() {}

void SceneManager::Init(EntityManager* entityMgr) {
    m_pEntityMgr = entityMgr;
    printf("[SceneManager] Initialized.\n");
}

// ============================================================
// 关卡加载
// ============================================================
void SceneManager::LoadLevel(int level, BiomeType biome, EntityManager& entityMgr) {
    m_currentLevel = level;
    m_currentBiome = biome;

    // 生成地图
    m_map->GenerateLevel(level, biome);

    // 加载起始房间
    const RoomData* startRoom = m_map->GetStartRoom();
    if (startRoom) {
        m_map->SetCurrentRoom(startRoom->coord);
        LoadRoom(startRoom->coord, entityMgr);
    }

    printf("[SceneManager] Level %d loaded. Biome=%d\n", level, (int)biome);
}

void SceneManager::UnloadLevel(EntityManager& entityMgr) {
    UnloadRoom(entityMgr);
    m_map.reset();
    m_map = std::unique_ptr<MapGenerator>(new MapGenerator());
    printf("[SceneManager] Level unloaded.\n");
}

// ============================================================
// 加载单个房间
// ============================================================
void SceneManager::LoadRoom(const RoomCoord& coord, EntityManager& entityMgr) {
    RoomData* room = m_map->GetRoomMutable(coord);
    if (!room) {
        printf("[SceneManager] LoadRoom: invalid room (%d,%d)\n", coord.col, coord.row);
        return;
    }

    printf("[SceneManager] Loading room (%d,%d) type=%d\n",
           coord.col, coord.row, (int)room->type);

    room->isDiscovered = true;

    // 已清过的房间不再刷怪，但商店和奖励房每次进入都要交互
    if (room->isCleared && room->type != RoomType::SHOP && room->type != RoomType::REWARD) {
        OpenAllDoors(entityMgr);
        UpdateDoorStates(entityMgr);
        printf("[SceneManager] Room (%d,%d) already cleared, skipping spawn.\n",
               coord.col, coord.row);
        return;
    }

    // 商店/奖励房再次进入时重新触发
    if (room->isCleared && room->type == RoomType::SHOP) {
        OpenAllDoors(entityMgr);
        UpdateDoorStates(entityMgr);
        SpawnShopItems(Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f), entityMgr);
        return;
    }
    if (room->isCleared && room->type == RoomType::REWARD) {
        OpenAllDoors(entityMgr);
        UpdateDoorStates(entityMgr);
        // 奖励房宝箱只开一次，不再重新生成
        return;
    }

    // 根据房间类型处理
    switch (room->type) {
        case RoomType::START:
            // 起始房间：无怪，门初始关闭
            CloseAllDoors(entityMgr);
            SpawnObstacles(entityMgr);
            break;

        case RoomType::NORMAL:
            CloseAllDoors(entityMgr);
            SpawnObstacles(entityMgr);
            SpawnRoomEnemies(*room, entityMgr);
            break;

        case RoomType::ELITE:
            CloseAllDoors(entityMgr);
            SpawnObstacles(entityMgr);
            SpawnEliteEnemies(*room, entityMgr);
            break;

        case RoomType::EXIT:
            CloseAllDoors(entityMgr);
            SpawnObstacles(entityMgr);
            SpawnEliteEnemies(*room, entityMgr);
            break;

        case RoomType::REWARD:
            // 奖励房：无怪，中央宝箱
            CloseAllDoors(entityMgr);
            SpawnChest(Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f),
                       entityMgr, RoomType::REWARD);
            break;

        case RoomType::SHOP:
            // 商店房：无怪，3 个商品
            CloseAllDoors(entityMgr);
            SpawnShopItems(Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f),
                           entityMgr);
            break;

        case RoomType::BOSS:
            CloseAllDoors(entityMgr);
            // Boss 在房间中央生成
            SpawnBoss(Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f - 40.0f),
                      entityMgr);
            break;

        default:
            break;
    }

    // 无敌人波次的房间（START/REWARD/SHOP），直接标记波次完成
    if (room->totalWaveCount == 0) {
        room->allWavesSpawned = true;
    }

    // 生成门（根据房间的门连接配置）
    UpdateDoorStates(entityMgr);
}

// ============================================================
// 卸载当前房间（保留玩家）
// ============================================================
void SceneManager::UnloadRoom(EntityManager& entityMgr) {
    // 移除所有非玩家实体
    auto& entities = const_cast<std::vector<std::unique_ptr<GameObject>>&>(
        entityMgr.GetAllEntities());

    for (auto& entity : entities) {
        if (!entity) continue;
        // 保留玩家
        Player* player = dynamic_cast<Player*>(entity.get());
        if (player) continue;
        entity->MarkForDeletion();
    }
    entityMgr.CleanupDeleted();
}

// ============================================================
// 房间切换过渡
// ============================================================
void SceneManager::TransitionToRoom(const RoomCoord& from, const RoomCoord& to) {
    if (m_isTransitioning) return;
    if (m_doorCooldown > 0.0f) return;  // 刚切换完房间，冷却中

    // 验证目标房间存在且连通
    if (!m_map->IsRoomConnected(from, to)) {
        printf("[SceneManager] Rooms not connected: (%d,%d) -> (%d,%d)\n",
               from.col, from.row, to.col, to.row);
        return;
    }

    m_transitionFrom = from;
    m_transitionTo = to;
    m_isTransitioning = true;
    m_transitionTimer = 0.0f;

    printf("[SceneManager] Transitioning: (%d,%d) -> (%d,%d)\n",
           from.col, from.row, to.col, to.row);
}

// ============================================================
// 每帧更新
// ============================================================
void SceneManager::Update(float deltaTime, EntityManager& entityMgr) {
    // 过渡动画
    if (m_isTransitioning) {
        m_transitionTimer += deltaTime;
        if (m_transitionTimer >= TRANSITION_DURATION) {
            // 过渡完成，切换房间
            m_isTransitioning = false;

            // 卸载旧房间
            UnloadRoom(entityMgr);

            // 切换到新房间
            m_map->SetCurrentRoom(m_transitionTo);

            // 将玩家移到新房间入口（根据来向的门）
            Player* player = GameManager::GetInstance().GetPlayer();
            if (player) {
                // 根据从哪扇门进来的，放置玩家在对应位置
                DoorDirection entryDir = DoorDirection::NONE;
                RoomCoord from = m_transitionFrom;
                RoomCoord to = m_transitionTo;
                if (to.col < from.col) entryDir = DoorDirection::WEST;
                else if (to.col > from.col) entryDir = DoorDirection::EAST;
                else if (to.row < from.row) entryDir = DoorDirection::NORTH;
                else if (to.row > from.row) entryDir = DoorDirection::SOUTH;

                const float margin = 100.0f;
                switch (entryDir) {
                    case DoorDirection::NORTH:
                        player->SetPosition(ROOM_WIDTH / 2.0f, ROOM_HEIGHT - margin);
                        break;
                    case DoorDirection::SOUTH:
                        player->SetPosition(ROOM_WIDTH / 2.0f, margin);
                        break;
                    case DoorDirection::EAST:
                        player->SetPosition(margin, ROOM_HEIGHT / 2.0f);
                        break;
                    case DoorDirection::WEST:
                        player->SetPosition(ROOM_WIDTH - margin, ROOM_HEIGHT / 2.0f);
                        break;
                    default:
                        player->SetPosition(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);
                        break;
                }
            }

            // 加载新房间
            LoadRoom(m_transitionTo, entityMgr);

            // 启动门交互冷却，防止立即触发另一扇门
            m_doorCooldown = DOOR_COOLDOWN;

            printf("[SceneManager] Transition complete. Now in room (%d,%d)\n",
                   m_transitionTo.col, m_transitionTo.row);
        }
        return;  // 过渡期间不进行其他更新
    }

    // 门交互冷却计时
    if (m_doorCooldown > 0.0f) {
        m_doorCooldown -= deltaTime;
    }

    // 波次调度
    RoomData* room = GetCurrentRoomMutable();
    if (!room) return;

    if (!room->allWavesSpawned && room->totalWaveCount > 0) {
        room->waveTimer += deltaTime;

        if (room->currentWave == 0 && room->waveTimer >= 1.0f) {
            // 首次进入房间后 1 秒刷第一波
            SpawnNextWave(entityMgr);
            room->waveTimer = 0.0f;
        } else if (room->currentWave > 0 && room->waveTimer >= m_waveCooldown) {
            SpawnNextWave(entityMgr);
            room->waveTimer = 0.0f;
        }
    }

    // 检查房间是否清空
    if (!room->isCleared) {
        bool allDead = entityMgr.AreAllEnemiesDead();
        if (allDead && room->allWavesSpawned) {
            room->isCleared = true;
            printf("[SceneManager] Room (%d,%d) CLEARED!\n",
                   room->coord.col, room->coord.row);

            // Boss 房间清除后触发天赋选择（第3关之后直接胜利，不选天赋）
            if (room->type == RoomType::BOSS) {
                GameManager::GetInstance().SetState(GameState::TALENT_SELECT);
            } else {
                // 打开所有门
                OpenAllDoors(entityMgr);
            }
        }
    }

    // 门状态同步
    UpdateDoorStates(entityMgr);
}

// ============================================================
// 房间数据
// ============================================================
const RoomData* SceneManager::GetCurrentRoom() const {
    return m_map->GetCurrentRoom();
}

RoomData* SceneManager::GetCurrentRoomMutable() {
    return m_map->GetCurrentRoomMutable();
}

// ============================================================
// 波次系统
// ============================================================
void SceneManager::SpawnNextWave(EntityManager& entityMgr) {
    RoomData* room = GetCurrentRoomMutable();
    if (!room) return;
    if (room->allWavesSpawned) return;

    room->currentWave++;
    printf("[SceneManager] Spawning wave %d/%d in room (%d,%d)\n",
           room->currentWave, room->totalWaveCount,
           room->coord.col, room->coord.row);

    int enemyCount = 0;
    int stageInBiome = ((m_currentLevel - 1) % 3) + 1;
    int biomeDepth = (m_currentLevel - 1) / 3;
    int levelScale = stageInBiome + biomeDepth;  // 每张地图内递增，后期地图再整体上浮
    switch (room->type) {
        case RoomType::NORMAL:
            enemyCount = RandomInt(2, 4) + (levelScale - 1);  // Lv1:2-4 Lv2:3-5 Lv3:4-6
            break;
        case RoomType::ELITE:
            enemyCount = RandomInt(3, 5) + (levelScale - 1);  // Lv1:3-5 Lv2:4-6 Lv3:5-7
            break;
        case RoomType::EXIT:
            enemyCount = RandomInt(4, 6) + (levelScale - 1);
            break;
        case RoomType::BOSS:
            enemyCount = 0;  // Boss 单独生成
            break;
        default:
            enemyCount = 0;
            break;
    }
    if (m_currentBiome == BiomeType::ICE_DUNGEON && room->type != RoomType::BOSS) {
        enemyCount += 1;
    }

    Player* player = GameManager::GetInstance().GetPlayer();
    float minSpawnDist = 250.0f;  // 距离玩家至少 250 像素

    for (int i = 0; i < enemyCount; ++i) {
        float margin = 80.0f;
        Vector2 spawnPos;
        int attempts = 0;
        bool validSpawn = false;

        // 最多尝试 20 次找到远离玩家的边缘位置
        while (attempts < 20) {
            int edge = RandomInt(0, 3);
            switch (edge) {
                case 0:  // 上边缘
                    spawnPos = Vector2(RandomFloat(margin, ROOM_WIDTH - margin), margin);
                    break;
                case 1:  // 下边缘
                    spawnPos = Vector2(RandomFloat(margin, ROOM_WIDTH - margin), ROOM_HEIGHT - margin);
                    break;
                case 2:  // 左边缘
                    spawnPos = Vector2(margin, RandomFloat(margin, ROOM_HEIGHT - margin));
                    break;
                default: // 右边缘
                    spawnPos = Vector2(ROOM_WIDTH - margin, RandomFloat(margin, ROOM_HEIGHT - margin));
                    break;
            }
            if (!player || Vector2::Distance(spawnPos, player->GetPosition()) >= minSpawnDist) {
                validSpawn = true;
                break;
            }
            attempts++;
        }

        bool isElite = (room->type == RoomType::ELITE);
        Enemy* enemy = SpawnEnemy(spawnPos, entityMgr, isElite);

        // 设置目标为玩家
        if (enemy) {
            enemy->SetTarget(player);
        }
    }

    // 检查是否所有波次已生成
    if (room->currentWave >= room->totalWaveCount) {
        room->allWavesSpawned = true;
    }
}

bool SceneManager::IsWaveComplete() const {
    const RoomData* room = GetCurrentRoom();
    if (!room) return true;
    if (!m_pEntityMgr) return true;
    return m_pEntityMgr->AreAllEnemiesDead();
}

bool SceneManager::IsRoomCleared() const {
    const RoomData* room = GetCurrentRoom();
    if (!room) return false;
    return room->isCleared;
}

// ============================================================
// 生成敌人
// ============================================================
Enemy* SceneManager::SpawnEnemy(const Vector2& position, EntityManager& entityMgr,
                                 bool isElite) {
    Enemy* enemy = CreateRandomEnemyForBiome(m_currentBiome, isElite);
    if (!enemy) return nullptr;

    // 关卡越高敌人血量越多：每个地图三小关递增，同时冰原/火山有整体难度抬升
    int stageInBiome = ((m_currentLevel - 1) % 3) + 1;
    int biomeDepth = (m_currentLevel - 1) / 3;
    float hpScale = 1.0f + (stageInBiome - 1) * 0.22f + biomeDepth * 0.26f;
    if (hpScale > 1.01f) {
        int boostedHP = static_cast<int>(enemy->GetMaxHP() * hpScale);
        enemy->SetMaxHP(boostedHP);
        enemy->SetHP(boostedHP);
    }

    enemy->SetPosition(position);
    entityMgr.RegisterEntity(std::unique_ptr<Enemy>(enemy));
    return enemy;
}

Boss* SceneManager::SpawnBoss(const Vector2& position, EntityManager& entityMgr) {
    Boss* boss = CreateBossForBiome(m_currentBiome);
    if (!boss) return nullptr;

    boss->SetPosition(position);
    boss->SetTarget(GameManager::GetInstance().GetPlayer());
    entityMgr.RegisterEntity(std::unique_ptr<Boss>(boss));
    printf("[SceneManager] Boss spawned: %s\n", boss->GetBossName());
    return boss;
}

// ============================================================
// 根据 Biome 随机创建敌人
// ============================================================
Enemy* SceneManager::CreateRandomEnemyForBiome(BiomeType biome, bool isElite) {
    Enemy* enemy = nullptr;

    switch (biome) {
        case BiomeType::FOREST: {
            int roll = RandomInt(0, 3);
            switch (roll) {
                case 0: enemy = new GoblinMelee();    break;
                case 1: enemy = new GoblinArcher();   break;
                case 2: enemy = new ChargeBoar();     break;
                case 3: enemy = new ForestWisp();     break;
            }
            break;
        }
        case BiomeType::ICE_DUNGEON: {
            int roll = RandomInt(0, 3);
            switch (roll) {
                case 0: enemy = new IceSlime();       break;
                case 1: enemy = new Snowman();        break;
                case 2: enemy = new IceMage();        break;
                case 3: enemy = new FrostScout();     break;
            }
            break;
        }
        case BiomeType::VOLCANO: {
            int roll = RandomInt(0, 3);
            switch (roll) {
                case 0: enemy = new LavaWorm();       break;
                case 1: enemy = new FireKnight();     break;
                case 2: enemy = new ExplosiveBat();   break;
                case 3: enemy = new EmberImp();       break;
            }
            break;
        }
    }

    if (enemy && isElite) {
        enemy->SetElite(true);
    }

    return enemy;
}

Boss* SceneManager::CreateBossForBiome(BiomeType biome) {
    switch (biome) {
        case BiomeType::FOREST:      return new Treant();
        case BiomeType::ICE_DUNGEON: return new CrystalCrab();
        case BiomeType::VOLCANO:     return new LavaDragon();
        default: return nullptr;
    }
}

// ============================================================
// 生成障碍物
// ============================================================
void SceneManager::SpawnObstacles(EntityManager& entityMgr) {
    RoomData* room = GetCurrentRoomMutable();
    if (!room) return;

    for (const Vector2& pos : room->obstaclePositions) {
        Obstacle* obs = nullptr;

        switch (m_currentBiome) {
            case BiomeType::FOREST:
                obs = new Tree();
                break;
            case BiomeType::ICE_DUNGEON:
            {
                int marker = ((int)pos.x * 31 + (int)pos.y * 17 + m_currentLevel * 13) % 100;
                obs = marker < 58 ? static_cast<Obstacle*>(new IceBlock())
                                  : static_cast<Obstacle*>(new IcePatch());
                break;
            }
            case BiomeType::VOLCANO:
                obs = new LavaPool();
                break;
        }

        if (obs) {
            obs->SetPosition(pos);
            if (LavaPool* lava = dynamic_cast<LavaPool*>(obs)) {
                lava->SetLifetime(99999.0f);
            }
            obs->SyncAABBToPosition();
            entityMgr.RegisterEntity(std::unique_ptr<Obstacle>(obs));
        }
    }
}

// ============================================================
// 门管理
// ============================================================
void SceneManager::OpenAllDoors(EntityManager& entityMgr) {
    auto doors = entityMgr.GetDoors();
    for (Door* door : doors) {
        if (door) {
            door->Unlock();
            door->Open();
        }
    }
}

void SceneManager::CloseAllDoors(EntityManager& entityMgr) {
    auto doors = entityMgr.GetDoors();
    for (Door* door : doors) {
        if (door) {
            door->Close();
        }
    }
}

void SceneManager::UpdateDoorStates(EntityManager& entityMgr) {
    RoomData* room = GetCurrentRoomMutable();
    if (!room) return;

    // 先移除旧的门
    auto doors = entityMgr.GetDoors();
    for (Door* door : doors) {
        if (door) door->MarkForDeletion();
    }
    entityMgr.CleanupDeleted();

    // 为房间的每个有效门方向创建门实体
    const float doorSize = 48.0f;
    const float dMargin = 24.0f;

    for (DoorDirection dir : room->activeDirections) {
        Vector2 doorPos;
        Door* door = entityMgr.SpawnEntity<Door>();

        switch (dir) {
            case DoorDirection::NORTH:
                doorPos = Vector2(ROOM_WIDTH / 2.0f, dMargin);
                break;
            case DoorDirection::SOUTH:
                doorPos = Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT - dMargin);
                break;
            case DoorDirection::EAST:
                doorPos = Vector2(ROOM_WIDTH - dMargin, ROOM_HEIGHT / 2.0f);
                break;
            case DoorDirection::WEST:
                doorPos = Vector2(dMargin, ROOM_HEIGHT / 2.0f);
                break;
            default:
                doorPos = Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);
                break;
        }

        door->SetPosition(doorPos);
        door->SetDoorDirection(dir);
        door->SetCollisionSize(doorSize / 2.0f, doorSize / 2.0f);
        door->SyncAABBToPosition();

        // 设置目标房间
        RoomCoord target = m_map->GetAdjacentRoom(room->coord, dir);
        door->SetTargetRoom(target);

        // 如果房间已清理，开门
        if (room->isCleared) {
            door->Unlock();
            door->Open();
        } else {
            door->Close();
        }
    }
}

// ============================================================
// 生成宝箱
// ============================================================
Chest* SceneManager::SpawnChest(const Vector2& position, EntityManager& entityMgr,
                                 RoomType sourceRoom) {
    Chest* chest = entityMgr.SpawnEntity<Chest>();
    chest->SetPosition(position);
    chest->SetSourceRoom(sourceRoom);

    int rollLevel = m_currentLevel;
    if (GameManager::GetInstance().GetBuffManager().HasTreasureInstinct()) rollLevel += 3;
    WeaponType weaponRoll = RollWeaponByTier(rollLevel, false);
    chest->SetAsWeaponDrop(weaponRoll);
    (void)sourceRoom;

    chest->SyncAABBToPosition();
    return chest;
}

// ============================================================
// 生成商店商品
// ============================================================
void SceneManager::SpawnShopItems(const Vector2& position, EntityManager& entityMgr) {
    (void)entityMgr;
    printf("[SceneManager] Generating shop items at (%.0f, %.0f)\n", position.x, position.y);

    GameManager& gm = GameManager::GetInstance();
    UIManager* ui = &gm.GetUIManager();

    // 每层首次进入商店时生成商品，后续访问复用
    if (ui->IsShopGeneratedThisLevel()) {
        printf("[SceneManager] Reusing cached shop items\n");
        ui->SetShopItems(ui->GetCachedShopItems());
        gm.SetState(GameState::SHOP);
        return;
    }

    Player* player = gm.GetPlayer();

    std::vector<ShopItemData> items;

    for (int i = 0; i < 2; ++i) {
        WeaponType wt = RollWeaponByTier(m_currentLevel, true);
        for (int attempt = 0; attempt < 12; ++attempt) {
            bool duplicateInShop = false;
            for (const auto& existing : items) {
                if (existing.itemType == ShopItemType::WEAPON && existing.weaponType == wt) {
                    duplicateInShop = true;
                    break;
                }
            }
            if (!duplicateInShop) break;
            wt = RollWeaponByTier(m_currentLevel, true);
        }

        bool alreadyOwned = false;
        if (player) {
            for (int s = 0; s < player->GetWeaponCount(); ++s) {
                Weapon* w = player->GetWeapon(s);
                if (w && w->GetType() == wt) { alreadyOwned = true; break; }
            }
        }
        if (alreadyOwned) wt = RollWeaponByTier(m_currentLevel, true);

        ShopItemData item;
        item.itemType = ShopItemType::WEAPON;
        item.weaponType = wt;
        item.isSold = false;
        item.name = GetWeaponDisplayName(wt);
        item.price = GetWeaponPrice(wt);
        items.push_back(item);
    }

    ShopItemData potion;
    potion.itemType = RandomInt(0, 1) == 0 ? ShopItemType::HP_POTION : ShopItemType::MP_POTION;
    potion.name = potion.itemType == ShopItemType::HP_POTION ? "急救药水" : "能量药水";
    potion.value = potion.itemType == ShopItemType::HP_POTION ? 3 : 60;
    potion.price = potion.itemType == ShopItemType::HP_POTION ? 45 : 38;
    potion.isSold = false;
    items.push_back(potion);

    // 缓存并标记
    ui->SetCachedShopItems(items);
    ui->SetShopGeneratedThisLevel(true);
    ui->SetShopItems(items);
    gm.SetState(GameState::SHOP);
}

// ============================================================
// 房间敌人生成
// ============================================================
void SceneManager::SpawnRoomEnemies(RoomData& room, EntityManager& entityMgr) {
    room.currentWave = 0;
    room.allWavesSpawned = false;
    room.waveTimer = 0.0f;
    room.isCleared = false;
}

void SceneManager::SpawnEliteEnemies(RoomData& room, EntityManager& entityMgr) {
    room.currentWave = 0;
    room.allWavesSpawned = false;
    room.waveTimer = 0.0f;
    room.isCleared = false;
}

// ============================================================
// 掉落物生成
// ============================================================
void SceneManager::SpawnEnemyDrops(const Vector2& position, EntityManager& entityMgr,
                                    bool isElite) {
    // 基础金币掉落
    int goldAmount = isElite ? RandomInt(8, 15) : RandomInt(2, 5);
    SpawnGoldDrop(position, goldAmount, entityMgr);

    // 精英额外掉落
    if (isElite) {
        // 15% 掉落血瓶
        if (RandomFloat(0.0f, 1.0f) < 0.15f) {
            DropItem* potion = entityMgr.SpawnEntity<DropItem>();
            potion->SetPosition(position.x + RandomFloat(-15.0f, 15.0f),
                                position.y + RandomFloat(-15.0f, 15.0f));
            potion->SetDropType(DropType::HP_POTION);
            potion->SetValue(2);
        }
    }
}

void SceneManager::SpawnGoldDrop(const Vector2& position, int amount,
                                  EntityManager& entityMgr) {
    // 将大额金币拆分为多个金币掉落物（每个最多 5 金币）
    int remaining = amount;
    while (remaining > 0) {
        int pile = (remaining > 5) ? RandomInt(3, 5) : remaining;
        remaining -= pile;

        DropItem* gold = entityMgr.SpawnEntity<DropItem>();
        gold->SetPosition(position.x + RandomFloat(-20.0f, 20.0f),
                          position.y + RandomFloat(-20.0f, 20.0f));
        gold->SetDropType(DropType::GOLD);
        gold->SetValue(pile);
    }
}
