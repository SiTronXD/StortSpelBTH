#pragma once 
#include "Room Handler.h"
#include "vengine/application/SceneHandler.hpp"
#include "vengine/ai/AIHandler.hpp"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"

#include <list>
#include <random>
#include <vector>


class GameScene;

using ImguiLambda = std::function<void(FSM* fsm, uint32_t entityId)>;



class TilePicker
{
private:

    std::mt19937 randomDev;

    std::vector<const TileInfo*> usedTiles;
    std::list<const TileInfo*> unusedTileInfos;
    std::map<const TileInfo*, bool> freeTiles; 
    std::map<uint32_t, const TileInfo*> ogNeighbourhood;

    glm::vec3 enemiesMidpoint;

    void calcEnemiesMidpoint();

    const TileInfo* getSpreadTile();

    std::vector<const TileInfo*> getPossibleNeighbours(const TileInfo* currentNeighbour,std::unordered_map<const TileInfo*, bool>& possibleNeigbhours);
    std::vector<const TileInfo*> getFreeNeighbours(const TileInfo* tile);

    void updateFreeTiles();

public:
    TilePicker() : randomDev(69){}//TODO: Use same seed as game...
        
    void   init(const std::vector<TileInfo>& freeTileInfos);
    size_t size() const;
    const TileInfo* getRandomEmptyTile();
    std::vector<const TileInfo*> getRandomEmptyNeighbouringTiles(const int nr);
    const TileInfo* getRandomFreeTileFarAwayFrom(const TileInfo* tile);

    void clean();
};

struct LichObjects
{
    uint32_t graveID;
    uint32_t alterID;
};

class SpawnHandler
{
public:
    inline static const int MAX_NR_OF_ENEMIES = 100;
    inline static const float PERCENTAGE_TANKS   = 0.10f;
    inline static const float PERCENTAGE_LICHS   = 0.25f;
    inline static const float PERCENTAGE_SWARMS  = 0.75f;

    const float enemiesPerTiles = 0.10f;

    inline static const int NR_BLOBS_IN_GROUP = 3;

    inline static const int MAX_NR_TANKS        = MAX_NR_OF_ENEMIES * PERCENTAGE_TANKS;
    inline static const int MAX_NR_LICHS        = MAX_NR_OF_ENEMIES * PERCENTAGE_LICHS;
    inline static const int MAX_NR_SWARMGROUPS  = (MAX_NR_OF_ENEMIES * PERCENTAGE_SWARMS)/NR_BLOBS_IN_GROUP;

    inline static const bool USE_DEBUG = true;
    inline static const int NR_TANK_DBG         = 0;
    inline static const int NR_LICH_DBG         = 2;
    inline static const int NR_SWARM_GROUPS_DBG = 0;

private:
    RoomHandler*     roomHandler      = nullptr;
    GameScene*       currScene        = nullptr;
    SceneHandler*    sceneHandler     = nullptr;
    AIHandler*       aiHandler        = nullptr;
    ResourceManager* resourceManager  = nullptr;
    UIRenderer*      uiRenderer       = nullptr;


    float nrOfEnemiesPerRoom = 0; // Set based on nr of Tile per room

    // Data for imgui
    int nrOfTilesInRoom = 0;

    // Enemies in current Room
    int nrOfTanks_inRoom  = 0;
    int nrOfLichs_inRoom  = 0;
    int nrOfSwarms_inRoom = 0;
    int nrOfGroups_inRoom = 0;


    std::vector<int> allEntityIDs;
    std::vector<int> swarmIDs;
    std::vector<int> lichIDs;
    std::vector<int> tankIDs;
    std::vector<SwarmGroup*> swarmGroups;

    std::unordered_map<uint32_t, LichObjects> lichObjects;

    struct pos_col{glm::vec3 pos; glm::vec3 col;};
    std::vector<pos_col> debugRays;

    uint32_t fontTextureIndex;

    TilePicker tilePicker;

    void spawnTank( const int tankIdx,  const glm::vec3& pos);
    uint32_t spawnLich( const int lichIdx, std::vector<const TileInfo*> tileInfos);
    uint32_t spawnSwarmGroup(const int swarmStartIdx, std::vector<const TileInfo*> tileInfo); //TODO: Do we need to have a vector of pos; say 2 to let a swarm spawn over two tiles?
    void spawnSwarm(const int swarmIdx, const glm::vec3& pos);
    void initTanks();

    void createTank();
    void createLich();
    void createSwarmGroup();

    ImguiLambda TankImgui ();
    ImguiLambda LichImgui ();
    ImguiLambda SwarmImgui();
public:   

    ~SpawnHandler()
    {
        for (auto& p : swarmGroups)
        {
            delete p;
        }
    }

    void init(RoomHandler* roomHandler, 
             GameScene* currentGameScene, 
             SceneHandler* sceneHandler, 
             AIHandler* aiHandler,
             ResourceManager* resourceManager,
             UIRenderer* uiRenderer)
    {
        this->roomHandler     = roomHandler;
        this->currScene       = currentGameScene;
        this->sceneHandler    = sceneHandler;
        this->aiHandler       = aiHandler;
        this->resourceManager = resourceManager;
        this->uiRenderer      = uiRenderer;
        this->createEntities();
    }

    void spawnEnemiesIntoRoom();

    void createEntities();

    void killAllEnemiesOutsideRoom();

    bool allDead();

    void updateImgui();
};