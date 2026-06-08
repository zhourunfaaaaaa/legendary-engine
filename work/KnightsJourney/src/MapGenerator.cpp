// ============================================================
// MapGenerator.cpp - 地图生成器（DFS 房间图 + 类型分配）
// ============================================================

#include "../include/System/MapGenerator.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>

MapGenerator::MapGenerator()
    : m_gridWidth(ROOM_COLS)
    , m_gridHeight(ROOM_ROWS)
    , m_biome(BiomeType::FOREST)
    , m_level(1)
    , m_normalRoomCount(0)
    , m_eliteRoomCount(0)
    , m_rewardRoomCount(0)
    , m_shopRoomCount(0) {

    // 初始化房间网格为 -1（无房间）
    for (int r = 0; r < ROOM_ROWS; ++r) {
        for (int c = 0; c < ROOM_COLS; ++c) {
            m_roomMap[r][c] = -1;
        }
    }
}

// ============================================================
// 生成一层完整地图
// ============================================================
void MapGenerator::GenerateLevel(int level, BiomeType biome) {
    m_level = level;
    m_biome = biome;
    m_rooms.clear();

    // 重置网格
    for (int r = 0; r < ROOM_ROWS; ++r) {
        for (int c = 0; c < ROOM_COLS; ++c) {
            m_roomMap[r][c] = -1;
            m_visited[r][c] = false;
        }
    }

    // 根据关卡数决定房间数量：前期 7 间，后期 8 间，保证完整一层节奏。
    int targetRoomCount = MIN_ROOMS + (level - 1) % 2;
    if (targetRoomCount > MAX_ROOMS) targetRoomCount = MAX_ROOMS;

    printf("[MapGenerator] Generating Level %d (Biome %d), target rooms: %d\n",
           level, (int)biome, targetRoomCount);

    // 1. DFS 生成连通房间图
    GenerateRoomGraph(targetRoomCount);

    // 2. 添加额外连接，增加地图网状结构
    AddExtraConnections();

    // 3. 分配房间类型
    AssignRoomTypes();

    // 3. 确保 Boss 在最远端
    PlaceBossRoom();

    // 4. 为每个房间生成障碍物
    for (auto& room : m_rooms) {
        GenerateObstaclesForRoom(room);
    }

    // 5. 设置当前房间为起始房间
    m_currentRoom = m_startRoom;

    printf("[MapGenerator] Map generated: %zu rooms, start=(%d,%d), boss=(%d,%d)\n",
           m_rooms.size(),
           m_startRoom.col, m_startRoom.row,
           m_bossRoom.col, m_bossRoom.row);
    PrintMap();
}

// ============================================================
// 生成网状地图：7-8 个房间，主路径 + 中后段侧分支 + 额外连接
// 布局：START → NORMAL → NORMAL/ELITE → ... → BOSS（主路径）
//       + SHOP、REWARD 挂载在主路径中后段，避免开局过早出现
// 目标：3-4 战斗房 + 1 精英房 + 1 宝箱房 + 1 商店 + 1 Boss
// 方向权重均衡，避免太直的路线；后期 AddExtraConnections 加环
// ============================================================
void MapGenerator::GenerateRoomGraph(int targetRoomCount) {
    const int dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    const DoorDirection doorDirs[4] = {
        DoorDirection::NORTH, DoorDirection::SOUTH,
        DoorDirection::WEST,  DoorDirection::EAST
    };

    // 从网格左侧中间开始
    int startCol = 1;
    int startRow = ROOM_ROWS / 2;

    // 创建 START 房间
    m_roomMap[startRow][startCol] = 0;
    m_visited[startRow][startCol] = true;
    RoomData startRoom;
    startRoom.coord = RoomCoord(startCol, startRow);
    startRoom.type = RoomType::START;
    m_rooms.push_back(startRoom);
    m_startRoom = startRoom.coord;
    int roomCount = 1;

    // ---- 主路径：随机游走，方向权重均衡 ----
    int mainCombat = targetRoomCount - 3; // 扣除 START、SHOP、REWARD、BOSS 后的主路径战斗段
    if (mainCombat < 3) mainCombat = 3;
    int goalMainPath = mainCombat + 1; // +1 给 BOSS

    RoomCoord prev = m_startRoom;
    std::vector<RoomCoord> mainPathRooms;
    mainPathRooms.push_back(m_startRoom);

    // 方向权重：略偏东，但不极端（35/25/25/15）
    for (int step = 0; step < goalMainPath; ++step) {
        struct Candidate { int dc, dr; int dirIdx; int weight; };
        std::vector<Candidate> candidates;

        int allDirs[4][3] = { {1,0,0}, {0,1,1}, {0,-1,2}, {-1,0,3} };
        int weights[4] = { 35, 25, 25, 15 };  // 东/南/北/西

        for (int d = 0; d < 4; ++d) {
            int nc = prev.col + allDirs[d][0];
            int nr = prev.row + allDirs[d][1];
            if (nc < 0 || nc >= ROOM_COLS || nr < 0 || nr >= ROOM_ROWS) continue;
            if (m_visited[nr][nc]) continue;
            candidates.push_back({allDirs[d][0], allDirs[d][1],
                                  allDirs[d][2], weights[d]});
        }

        if (candidates.empty()) {
            printf("[MapGenerator] Warning: dead end at step %d\n", step);
            break;
        }

        // 加权随机选择
        int totalWeight = 0;
        for (auto& c : candidates) totalWeight += c.weight;
        int roll = RandomInt(1, totalWeight);
        int cumulative = 0;
        size_t pick = 0;
        for (size_t i = 0; i < candidates.size(); ++i) {
            cumulative += candidates[i].weight;
            if (roll <= cumulative) { pick = i; break; }
        }

        int nc = prev.col + candidates[pick].dc;
        int nr = prev.row + candidates[pick].dr;
        int dirIdx = candidates[pick].dirIdx;

        m_roomMap[nr][nc] = roomCount;
        m_visited[nr][nc] = true;

        RoomData room;
        room.coord = RoomCoord(nc, nr);

        int stageInBiome = ((m_level - 1) % 3) + 1;
        if (step == goalMainPath - 1) {
            room.type = stageInBiome == 3 ? RoomType::BOSS : RoomType::EXIT;
            room.totalWaveCount = stageInBiome == 3 ? 1 : RandomInt(2, 3);
        } else if (step == goalMainPath - 2) {
            room.type = RoomType::ELITE;
            room.totalWaveCount = RandomInt(2, 3);
        } else {
            room.type = RoomType::NORMAL;
            room.totalWaveCount = RandomInt(2, 3);
        }
        m_rooms.push_back(room);

        DoorDirection dirFromPrev;
        switch (dirIdx) {
            case 0: dirFromPrev = DoorDirection::EAST;  break;
            case 1: dirFromPrev = DoorDirection::SOUTH; break;
            case 2: dirFromPrev = DoorDirection::NORTH; break;
            case 3: dirFromPrev = DoorDirection::WEST;  break;
            default: dirFromPrev = DoorDirection::EAST; break;
        }
        ConnectRooms(prev, RoomCoord(nc, nr), dirFromPrev);
        mainPathRooms.push_back(RoomCoord(nc, nr));
        roomCount++;
        prev = RoomCoord(nc, nr);
    }

    // ---- 侧分支：每张地图3小关共享 1 商店 + 2 宝箱房 ----
    int branchStart = (std::max)(2, (int)mainPathRooms.size() / 2);
    int branchEnd = (int)mainPathRooms.size() - 2;
    if (branchEnd < branchStart) branchEnd = branchStart;

    static bool planReady[3] = { false, false, false };
    static int shopStage[3] = { 1, 2, 3 };
    static int rewardSkipStage[3] = { 3, 1, 2 };
    int biomeIndex = (m_level - 1) / 3;
    if (biomeIndex < 0) biomeIndex = 0;
    if (biomeIndex > 2) biomeIndex = 2;
    int stageInBiome = ((m_level - 1) % 3) + 1;
    if (!planReady[biomeIndex]) {
        shopStage[biomeIndex] = RandomInt(1, 3);
        rewardSkipStage[biomeIndex] = RandomInt(1, 3);
        planReady[biomeIndex] = true;
    }

    std::vector<RoomType> branchTypes;
    if (stageInBiome == shopStage[biomeIndex]) branchTypes.push_back(RoomType::SHOP);
    if (stageInBiome != rewardSkipStage[biomeIndex]) branchTypes.push_back(RoomType::REWARD);
    if (branchTypes.size() > 1 && RandomInt(0, 1) == 1) {
        std::swap(branchTypes[0], branchTypes[1]);
    }

    for (int b = 0; b < (int)branchTypes.size(); ++b) {
        if (roomCount >= targetRoomCount) break;

        // 尝试每个主路径房间作为挂载点（从分散的开始）
        bool placed = false;
        int firstTry = branchStart + b * ((std::max)(1, branchEnd - branchStart + 1) / 2);
        if (firstTry > branchEnd) firstTry = branchEnd;

        for (int attempt = 0; attempt <= (branchEnd - branchStart); ++attempt) {
            int baseIdx = firstTry;
            if (attempt % 2 == 0) baseIdx = firstTry + attempt / 2;
            else baseIdx = firstTry - (attempt + 1) / 2;
            if (baseIdx < branchStart || baseIdx > branchEnd) continue;

            RoomCoord baseCoord = mainPathRooms[baseIdx];

            // 尝试所有4个方向
            int order[4] = { 0, 1, 2, 3 };
            for (int i = 0; i < 4; ++i) { int j = RandomInt(0, 3); int t = order[i]; order[i] = order[j]; order[j] = t; }

            for (int i = 0; i < 4; ++i) {
                int d = order[i];
                int nc = baseCoord.col + dirs[d][0];
                int nr = baseCoord.row + dirs[d][1];
                if (nc < 0 || nc >= ROOM_COLS || nr < 0 || nr >= ROOM_ROWS) continue;
                if (m_visited[nr][nc]) continue;

                m_roomMap[nr][nc] = roomCount;
                m_visited[nr][nc] = true;

                RoomData branchRoom;
                branchRoom.coord = RoomCoord(nc, nr);
                branchRoom.type = branchTypes[b];
                branchRoom.totalWaveCount = 0;
                m_rooms.push_back(branchRoom);

                ConnectRooms(baseCoord, RoomCoord(nc, nr), doorDirs[d]);
                roomCount++;
                placed = true;
                break;
            }
            if (placed) break;
        }

        if (!placed) {
            printf("[MapGenerator] Could not place branch %d after all attempts\n", b);
            // 回退：将主路径中后段的 NORMAL 改为此类型
            for (int i = branchEnd; i >= branchStart; --i) {
                RoomData* r = GetRoomMutable(mainPathRooms[i]);
                if (r && r->type == RoomType::NORMAL) {
                    r->type = branchTypes[b];
                    r->totalWaveCount = 0;
                    printf("[MapGenerator] Converted NORMAL at (%d,%d) to branch type\n",
                           r->coord.col, r->coord.row);
                    break;
                }
            }
        }
    }

    printf("[MapGenerator] Mesh layout: %d rooms (main %zu + branches)\n",
           roomCount, mainPathRooms.size());
}

// ============================================================
// 在两个相邻房间之间建立门
// ============================================================
void MapGenerator::ConnectRooms(const RoomCoord& a, const RoomCoord& b,
                                 DoorDirection dirFromA) {
    RoomData* roomA = GetRoomMutable(a);
    RoomData* roomB = GetRoomMutable(b);
    if (!roomA || !roomB) return;

    int idxA = static_cast<int>(dirFromA);
    DoorDirection dirFromB = OppositeDirection(dirFromA);
    int idxB = static_cast<int>(dirFromB);

    roomA->doors[idxA] = true;
    roomB->doors[idxB] = true;

    // 记录有效门方向
    if (std::find(roomA->activeDirections.begin(), roomA->activeDirections.end(), dirFromA)
        == roomA->activeDirections.end()) {
        roomA->activeDirections.push_back(dirFromA);
    }
    if (std::find(roomB->activeDirections.begin(), roomB->activeDirections.end(), dirFromB)
        == roomB->activeDirections.end()) {
        roomB->activeDirections.push_back(dirFromB);
    }
}

// ============================================================
// 为单门房间添加额外连接，避免死胡同
// ============================================================
void MapGenerator::AddExtraConnections() {
    const int dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    const DoorDirection doorDirs[4] = {
        DoorDirection::NORTH, DoorDirection::SOUTH,
        DoorDirection::WEST,  DoorDirection::EAST
    };

    for (auto& room : m_rooms) {
        // 只处理非 START、非 BOSS 且只有 1 扇门的房间
        if (room.type == RoomType::START || room.type == RoomType::BOSS ||
            room.type == RoomType::EXIT ||
            room.type == RoomType::SHOP || room.type == RoomType::REWARD)
            continue;
        if (room.activeDirections.size() > 1)
            continue;

        // 随机尝试各方向
        int order[4] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; ++i) {
            int j = RandomInt(0, 3);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }

        for (int i = 0; i < 4; ++i) {
            int d = order[i];
            int nc = room.coord.col + dirs[d][0];
            int nr = room.coord.row + dirs[d][1];

            if (nc < 0 || nc >= ROOM_COLS || nr < 0 || nr >= ROOM_ROWS)
                continue;

            // 检查是否已与这个方向的邻居连通
            bool alreadyConnected = false;
            for (DoorDirection existDir : room.activeDirections) {
                RoomCoord adj = GetAdjacentRoom(room.coord, existDir);
                if (adj.col == nc && adj.row == nr) { alreadyConnected = true; break; }
            }
            if (alreadyConnected) continue;

            RoomCoord neighbor(nc, nr);
            RoomData* neighborRoom = GetRoomMutable(neighbor);

            // 不与起始房间建立额外连接，保持起始房间只有一扇门
            if (neighborRoom && neighborRoom->type == RoomType::START) continue;

            if (!neighborRoom) {
                // 相邻格子没有房间，创建新房间并连接
                if ((int)m_rooms.size() >= MAX_ROOMS) continue;

                int idx = (int)m_rooms.size();
                m_roomMap[nr][nc] = idx;
                m_visited[nr][nc] = true;

                RoomData newRoom;
                newRoom.coord = neighbor;
                newRoom.type = RoomType::NORMAL;
                newRoom.totalWaveCount = RandomInt(1, 3);
                m_rooms.push_back(newRoom);
                m_normalRoomCount++;

                ConnectRooms(room.coord, neighbor, doorDirs[d]);
                printf("[MapGenerator] Extra room+connection: (%d,%d) <-> (%d,%d)\n",
                       room.coord.col, room.coord.row, nc, nr);
                break;
            } else {
                // 已有房间，直接连接
                ConnectRooms(room.coord, neighbor, doorDirs[d]);
                printf("[MapGenerator] Extra connection: (%d,%d) <-> (%d,%d)\n",
                       room.coord.col, room.coord.row, nc, nr);
                break;
            }
        }
    }

}

DoorDirection MapGenerator::OppositeDirection(DoorDirection dir) {
    switch (dir) {
        case DoorDirection::NORTH: return DoorDirection::SOUTH;
        case DoorDirection::SOUTH: return DoorDirection::NORTH;
        case DoorDirection::EAST:  return DoorDirection::WEST;
        case DoorDirection::WEST:  return DoorDirection::EAST;
        default: return DoorDirection::NONE;
    }
}

// ============================================================
// 分配房间类型
// GenerateRoomGraph 已预设主路径类型，此处仅处理 EMPTY 占位房
// ============================================================
void MapGenerator::AssignRoomTypes() {
    // 统计各类型计数
    m_normalRoomCount = 0;
    m_eliteRoomCount = 0;
    m_rewardRoomCount = 0;
    m_shopRoomCount = 0;

    for (auto& room : m_rooms) {
        // EMPTY 占位房随机变为 SHOP 或 REWARD
        if (room.type == RoomType::EMPTY) {
            if (RandomFloat(0.0f, 1.0f) < 0.5f) {
                room.type = RoomType::SHOP;
                room.totalWaveCount = 0;
            } else {
                room.type = RoomType::REWARD;
                room.totalWaveCount = 0;
            }
        }

        // 更新计数
        switch (room.type) {
            case RoomType::NORMAL: m_normalRoomCount++; break;
            case RoomType::ELITE:  m_eliteRoomCount++;  break;
            case RoomType::REWARD: m_rewardRoomCount++; break;
            case RoomType::SHOP:   m_shopRoomCount++;   break;
            default: break;
        }

        // START 房间没有波次
        if (room.type == RoomType::START) {
            room.totalWaveCount = 0;
        }
    }

}

// ============================================================
// 确保 Boss 房间坐标已记录（GenerateRoomGraph 已预设类型）
// ============================================================
void MapGenerator::PlaceBossRoom() {
    for (auto& room : m_rooms) {
        if (room.type == RoomType::BOSS) {
            m_bossRoom = room.coord;
            return;
        }
    }
    for (auto& room : m_rooms) {
        if (room.type == RoomType::EXIT) {
            m_bossRoom = room.coord;
            return;
        }
    }
    // 回退：没有预设终点，选最远的非 START 房间作为出口
    int furthestIdx = -1;
    int maxDist = 0;
    for (int i = 0; i < (int)m_rooms.size(); ++i) {
        if (m_rooms[i].type == RoomType::START) continue;
        int dist = abs(m_rooms[i].coord.col - m_startRoom.col) +
                  abs(m_rooms[i].coord.row - m_startRoom.row);
        if (dist > maxDist) { maxDist = dist; furthestIdx = i; }
    }
    if (furthestIdx >= 0) {
        m_rooms[furthestIdx].type = RoomType::EXIT;
        m_rooms[furthestIdx].totalWaveCount = RandomInt(2, 3);
        m_bossRoom = m_rooms[furthestIdx].coord;
    }
}

// ============================================================
// 生成房间内障碍物（根据 Biome）
// ============================================================
void MapGenerator::GenerateObstaclesForRoom(RoomData& room) {
    room.obstaclePositions.clear();

    // START、REWARD、SHOP 房间不生成障碍物
    if (room.type == RoomType::START ||
        room.type == RoomType::REWARD ||
        room.type == RoomType::SHOP) {
        return;
    }

    int obstacleCount = 0;
    switch (m_biome) {
        case BiomeType::FOREST:
            obstacleCount = RandomInt(3, 6);    // 树木较多
            break;
        case BiomeType::ICE_DUNGEON:
            obstacleCount = RandomInt(5, 8);    // 冰晶柱 + 冰地板，第二地图更有地形压迫
            break;
        case BiomeType::VOLCANO:
            obstacleCount = RandomInt(2, 5);    // 岩浆适中
            break;
    }

    // BOSS 房间减少障碍物
    if (room.type == RoomType::BOSS) {
        obstacleCount = (m_biome == BiomeType::ICE_DUNGEON) ? RandomInt(2, 4) : RandomInt(0, 2);
    }

    const float margin = 100.0f;  // 边缘留空
    const float centerMargin = 80.0f;  // 中心出生点留空

    for (int i = 0; i < obstacleCount; ++i) {
        Vector2 pos;
        int attempts = 0;
        bool valid = false;

        while (attempts < 20) {
            pos.x = RandomFloat(margin, ROOM_WIDTH - margin);
            pos.y = RandomFloat(margin, ROOM_HEIGHT - margin);

            // 避免堵在出生点（房间中心）
            float distToCenter = Vector2::Distance(pos,
                Vector2(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f));
            if (distToCenter < centerMargin) {
                attempts++;
                continue;
            }

            // 避免与其他障碍物重叠
            bool overlap = false;
            for (const auto& existing : room.obstaclePositions) {
                if (Vector2::Distance(pos, existing) < 60.0f) {
                    overlap = true;
                    break;
                }
            }
            if (overlap) {
                attempts++;
                continue;
            }

            valid = true;
            break;
        }

        if (valid) {
            room.obstaclePositions.push_back(pos);
        }
    }
}

// ============================================================
// 房间数据查询
// ============================================================
const RoomData* MapGenerator::GetRoom(const RoomCoord& coord) const {
    int idx = m_roomMap[coord.row][coord.col];
    if (idx < 0 || idx >= (int)m_rooms.size()) return nullptr;
    // 安全校验：确保 m_rooms[idx] 的坐标匹配
    if (m_rooms[idx].coord.col == coord.col && m_rooms[idx].coord.row == coord.row) {
        return &m_rooms[idx];
    }
    // fallback: 线性查找
    for (const auto& room : m_rooms) {
        if (room.coord == coord) return &room;
    }
    return nullptr;
}

RoomData* MapGenerator::GetRoomMutable(const RoomCoord& coord) {
    int idx = m_roomMap[coord.row][coord.col];
    if (idx >= 0 && idx < (int)m_rooms.size()) {
        if (m_rooms[idx].coord == coord) return &m_rooms[idx];
    }
    for (auto& room : m_rooms) {
        if (room.coord == coord) return &room;
    }
    return nullptr;
}

void MapGenerator::SetCurrentRoom(const RoomCoord& coord) {
    m_currentRoom = coord;
}

bool MapGenerator::MoveToRoom(DoorDirection direction) {
    RoomCoord adj = GetAdjacentRoom(m_currentRoom, direction);
    RoomData* room = GetRoomMutable(adj);
    if (room) {
        m_currentRoom = adj;
        return true;
    }
    return false;
}

RoomCoord MapGenerator::GetAdjacentRoom(const RoomCoord& coord,
                                         DoorDirection dir) const {
    switch (dir) {
        case DoorDirection::NORTH: return RoomCoord(coord.col, coord.row - 1);
        case DoorDirection::SOUTH: return RoomCoord(coord.col, coord.row + 1);
        case DoorDirection::EAST:  return RoomCoord(coord.col + 1, coord.row);
        case DoorDirection::WEST:  return RoomCoord(coord.col - 1, coord.row);
        default: return coord;
    }
}

const RoomData* MapGenerator::GetStartRoom() const {
    return GetRoom(m_startRoom);
}

const RoomData* MapGenerator::GetBossRoom() const {
    return GetRoom(m_bossRoom);
}

bool MapGenerator::IsRoomConnected(const RoomCoord& a, const RoomCoord& b) const {
    const RoomData* roomA = GetRoom(a);
    if (!roomA) return false;

    for (DoorDirection dir : roomA->activeDirections) {
        RoomCoord adj = GetAdjacentRoom(a, dir);
        if (adj == b) return true;
    }
    return false;
}

// ============================================================
// 世界坐标转换（每个房间使用相同的局部坐标空间）
// ============================================================
Vector2 MapGenerator::RoomToWorldPosition(const RoomCoord& coord) const {
    // 所有房间共享同一个屏幕坐标空间（0,0）到（ROOM_WIDTH, ROOM_HEIGHT）
    (void)coord;
    return Vector2(0.0f, 0.0f);
}

Vector2 MapGenerator::RoomLocalToWorld(const RoomCoord& coord,
                                        float localX, float localY) const {
    (void)coord;
    return Vector2(localX, localY);
}

AABB MapGenerator::GetRoomWorldBounds(const RoomCoord& coord) const {
    (void)coord;
    return AABB(ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f,
                ROOM_WIDTH / 2.0f, ROOM_HEIGHT / 2.0f);
}

// ============================================================
// 调试输出
// ============================================================
void MapGenerator::PrintMap() const {
    printf("[MapGenerator] === Level %d Map ===\n", m_level);
    for (int r = 0; r < ROOM_ROWS; ++r) {
        printf("  ");
        for (int c = 0; c < ROOM_COLS; ++c) {
            int idx = m_roomMap[r][c];
            if (idx < 0) {
                printf(" . ");
            } else {
                char typeChar = '?';
                switch (m_rooms[idx].type) {
                    case RoomType::START:  typeChar = 'S'; break;
                    case RoomType::NORMAL: typeChar = 'N'; break;
                    case RoomType::ELITE:  typeChar = 'E'; break;
                    case RoomType::REWARD: typeChar = 'R'; break;
                    case RoomType::SHOP:   typeChar = '$'; break;
                    case RoomType::BOSS:   typeChar = 'B'; break;
                    case RoomType::EXIT:   typeChar = 'X'; break;
                    default: break;
                }
                printf(" %c ", typeChar);
            }
        }
        printf("\n");
    }
    printf("  S=Start N=Normal E=Elite R=Reward $=Shop B=Boss X=Exit\n");
}
