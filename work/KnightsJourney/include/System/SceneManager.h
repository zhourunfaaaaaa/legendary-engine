#pragma once

#include "../Core/Common.h"
#include <vector>
#include <memory>

struct RoomData;
class MapGenerator;
class EntityManager;
class Player;

// ============================================================
// SceneManager - 场景管理器
// 负责当前房间的实体生成/销毁、波次调度、房间切换过渡
// ============================================================
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    // 初始化
    void Init(EntityManager* entityMgr);

    // 加载指定关卡
    void LoadLevel(int level, BiomeType biome, EntityManager& entityMgr);
    void UnloadLevel(EntityManager& entityMgr);

    // 加载/卸载单个房间
    void LoadRoom(const RoomCoord& coord, EntityManager& entityMgr);
    void UnloadRoom(EntityManager& entityMgr);

    // 切换房间（处理过渡动画）
    void TransitionToRoom(const RoomCoord& from, const RoomCoord& to);

    // 每帧更新（波次调度、房间状态）
    void Update(float deltaTime, EntityManager& entityMgr);

    // 获取当前地图
    MapGenerator* GetMap() { return m_map.get(); }
    const MapGenerator* GetMap() const { return m_map.get(); }

    // 获取当前房间数据
    const RoomData* GetCurrentRoom() const;
    RoomData* GetCurrentRoomMutable();

    // 波次系统
    void SpawnNextWave(EntityManager& entityMgr);
    bool IsWaveComplete() const;
    bool IsRoomCleared() const;

    // 生成特定类型的敌人（根据房间类型和 Biome）
    Enemy* SpawnEnemy(const Vector2& position, EntityManager& entityMgr, bool isElite = false);

    // 生成 Boss
    Boss* SpawnBoss(const Vector2& position, EntityManager& entityMgr);

    // 生成障碍物
    void SpawnObstacles(EntityManager& entityMgr);

    // 生成宝箱
    Chest* SpawnChest(const Vector2& position, EntityManager& entityMgr, RoomType sourceRoom);

    // 生成商店商品
    void SpawnShopItems(const Vector2& position, EntityManager& entityMgr);

    // 门的管理
    void OpenAllDoors(EntityManager& entityMgr);
    void CloseAllDoors(EntityManager& entityMgr);
    void UpdateDoorStates(EntityManager& entityMgr);

    // 掉落物生成
    void SpawnEnemyDrops(const Vector2& position, EntityManager& entityMgr, bool isElite);
    void SpawnGoldDrop(const Vector2& position, int amount, EntityManager& entityMgr);

    // 当前关卡信息
    BiomeType GetCurrentBiome() const { return m_currentBiome; }
    int GetCurrentLevel() const { return m_currentLevel; }

private:
    // 根据房间类型和关卡分配敌人组合
    void SpawnRoomEnemies(RoomData& room, EntityManager& entityMgr);
    void SpawnEliteEnemies(RoomData& room, EntityManager& entityMgr);

    // 根据 Biome 随机选择敌人类型
    Enemy* CreateRandomEnemyForBiome(BiomeType biome, bool isElite);
    Boss* CreateBossForBiome(BiomeType biome);

    std::unique_ptr<MapGenerator>   m_map;
    EntityManager*                  m_pEntityMgr;

    BiomeType   m_currentBiome;
    int         m_currentLevel;

    // 房间过渡
    bool        m_isTransitioning;
    float       m_transitionTimer;
    RoomCoord   m_transitionFrom;
    RoomCoord   m_transitionTo;
    static constexpr float TRANSITION_DURATION = 0.8f;

    // 门交互冷却：防止进入房间后立即触发另一扇门
    float       m_doorCooldown;
    static constexpr float DOOR_COOLDOWN = 0.5f;

    // 波次管理
    float       m_waveCooldown;     // 波与波之间的间隔
};
