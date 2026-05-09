#pragma once

#include "../Core/Common.h"
#include <vector>
#include <array>

// ============================================================
// RoomData - 单个房间的运行时数据
// ============================================================
struct RoomData {
    RoomCoord       coord;
    RoomType        type;
    bool            isCleared;          // 怪物是否已清完
    bool            isDiscovered;       // 玩家是否进入过
    bool            doors[4];           // N/S/E/W 门是否存在
    bool            doorsOpen[4];       // N/S/E/W 门是否开启
    std::vector<DoorDirection> activeDirections;  // 有效门方向

    // 房间内的敌人波次调度
    int             totalWaveCount;
    int             currentWave;
    bool            allWavesSpawned;
    float           waveTimer;          // 下一波计时

    // 房间内的障碍物位置（相对房间局部坐标）
    std::vector<Vector2> obstaclePositions;

    RoomData() : coord(0, 0), type(RoomType::EMPTY), isCleared(false),
                 isDiscovered(false), totalWaveCount(0), currentWave(0),
                 allWavesSpawned(false), waveTimer(0.0f) {
        for (int i = 0; i < 4; ++i) { doors[i] = false; doorsOpen[i] = false; }
    }
};

// ============================================================
// MapGenerator - 地图生成器
// 采用二维数组表示房间网格，使用随机 DFS/BSP 生成房间布局
// ============================================================
class MapGenerator {
public:
    MapGenerator();
    ~MapGenerator() = default;

    // 生成一层完整地图
    void GenerateLevel(int level, BiomeType biome);

    // 获取房间数据
    const RoomData* GetRoom(const RoomCoord& coord) const;
    RoomData* GetRoomMutable(const RoomCoord& coord);

    // 获取当前房间
    const RoomData* GetCurrentRoom() const { return GetRoom(m_currentRoom); }
    RoomData* GetCurrentRoomMutable() { return GetRoomMutable(m_currentRoom); }
    const RoomCoord& GetCurrentRoomCoord() const { return m_currentRoom; }

    // 切换当前房间
    void SetCurrentRoom(const RoomCoord& coord);
    bool MoveToRoom(DoorDirection direction);

    // 获取相邻房间
    RoomCoord GetAdjacentRoom(const RoomCoord& coord, DoorDirection dir) const;

    // 获取所有房间
    const std::vector<RoomData>& GetAllRooms() const { return m_rooms; }

    // 获取起始房间和 Boss 房间
    const RoomData* GetStartRoom() const;
    const RoomData* GetBossRoom() const;

    // 检查是否有房间在所有房间间连通
    bool IsRoomConnected(const RoomCoord& a, const RoomCoord& b) const;

    // 地图尺寸
    int GetGridWidth() const { return m_gridWidth; }
    int GetGridHeight() const { return m_gridHeight; }

    // 生成障碍物（根据 Biome）
    void GenerateObstaclesForRoom(RoomData& room);

    // 获取房间的世界坐标（像素）
    Vector2 RoomToWorldPosition(const RoomCoord& coord) const;
    Vector2 RoomLocalToWorld(const RoomCoord& coord, float localX, float localY) const;
    AABB   GetRoomWorldBounds(const RoomCoord& coord) const;

    // 调试
    void PrintMap() const;

private:
    // 生成房间图结构（DFS 生成连通图）
    void GenerateRoomGraph(int targetRoomCount);

    // 分配房间类型
    void AssignRoomTypes();

    // 确保 Boss 房间在所有路径末端
    void PlaceBossRoom();

    // 在两个相邻房间之间建立门
    void ConnectRooms(const RoomCoord& a, const RoomCoord& b, DoorDirection dirFromA);

    // 为单门房间（非 Start/Boss）添加额外连接，避免死胡同
    void AddExtraConnections();

    // 反方向
    static DoorDirection OppositeDirection(DoorDirection dir);

    int                     m_gridWidth;
    int                     m_gridHeight;
    RoomCoord               m_startRoom;
    RoomCoord               m_bossRoom;
    RoomCoord               m_currentRoom;
    BiomeType               m_biome;
    int                     m_level;

    // 房间网格（可用 roomMap[row][col] 访问）
    std::array<std::array<int, ROOM_COLS>, ROOM_ROWS> m_roomMap;

    // 所有房间列表
    std::vector<RoomData>   m_rooms;

    // DFS 访问标记
    std::array<std::array<bool, ROOM_COLS>, ROOM_ROWS> m_visited;

    // 房间类型计数
    int m_normalRoomCount;
    int m_eliteRoomCount;
    int m_rewardRoomCount;
    int m_shopRoomCount;

    static constexpr int MIN_ROOMS = 7;
    static constexpr int MAX_ROOMS = 8;
};
