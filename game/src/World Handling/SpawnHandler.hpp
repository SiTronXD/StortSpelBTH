#pragma once 
#include "Room Handler.h"
#include "vengine/application/SceneHandler.hpp"
#include "vengine/ai/AIHandler.hpp"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"
#include "../Components/AiElite.hpp"

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
    TilePicker() : randomDev(69){} //TODO: Use same seed as game...
        
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
    inline static const int NR_BLOBS_IN_GROUP = 3;


    inline static const float PERCENTAGE_TANKS   = 0.35f;
    inline static const float PERCENTAGE_LICHS   = 0.50f;
    inline static const float PERCENTAGE_SWARMS  = 0.80f * NR_BLOBS_IN_GROUP;

    const float MAX_ENEMIES_PER_TILES = 0.25f;
    const float MIN_ENEMIES_PER_TILES = 0.05f;

    inline static const int MAX_NR_TANKS        = (int)(MAX_NR_OF_ENEMIES * PERCENTAGE_TANKS);
    inline static const int MAX_NR_LICHS        = (int)(MAX_NR_OF_ENEMIES * PERCENTAGE_LICHS);
    inline static const int MAX_NR_SWARMGROUPS  = (int)((MAX_NR_OF_ENEMIES * PERCENTAGE_SWARMS)/NR_BLOBS_IN_GROUP/NR_BLOBS_IN_GROUP);

    inline static const bool USE_DEBUG = false;
    inline static const int NR_TANK_DBG         = 2;
    inline static const int NR_LICH_DBG         = 2;
    inline static const int NR_SWARM_GROUPS_DBG = 2;

private:
    RoomHandler*     roomHandler      = nullptr;
    Scene*           currScene        = nullptr;
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

    void        spawnTank        (const int tankIdx         , const glm::vec3& pos                  , int level, bool elite = false);
    uint32_t    spawnLich        (const int lichIdx         , std::vector<const TileInfo*> tileInfos, int level, bool elite = false);
    uint32_t    spawnSwarmGroup  (const int swarmStartIdx   , std::vector<const TileInfo*> tileInfo , int level, bool elite = false); //TODO: Do we need to have a vector of pos; say 2 to let a swarm spawn over two tiles?
    void        spawnSwarm       (const int swarmIdx        , const glm::vec3& pos                  , int level, bool elite = false);
    void        initTanks        ();

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
             //GameScene* currentGameScene, 
             Scene* currentGameScene, 
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

    void spawnEnemiesIntoRoom(int level);
    void resetEnemies();

    void createEntities();

    void killAllEnemiesOutsideRoom();

    bool allDead();

    void updateImgui();
};