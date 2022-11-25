#pragma once 
#include "Room Handler.h"
#include "vengine/application/SceneHandler.hpp"
#include "vengine/ai/AIHandler.hpp"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"


class GameScene;

using ImguiLambda = std::function<void(FSM* fsm, uint32_t entityId)>;

class SpawnHandler
{
public:
    inline static const int MAX_NR_OF_ENEMIES = 100;
    inline static const float PERCENTAGE_TANKS   = 0.10f;
    inline static const float PERCENTAGE_LICHS   = 0.25f;
    inline static const float PERCENTAGE_SWARMG  = 0.75f;

    const float enemiesPerTiles = 1.0;

    inline static const int NR_BLOBS_IN_GROUP = 3;

    inline static const int MAX_NR_TANKS        = MAX_NR_OF_ENEMIES * PERCENTAGE_TANKS;
    inline static const int MAX_NR_LICHS        = MAX_NR_OF_ENEMIES * PERCENTAGE_LICHS;
    inline static const int MAX_NR_SWARMGROUPS  = (MAX_NR_OF_ENEMIES * PERCENTAGE_SWARMG)/NR_BLOBS_IN_GROUP;
private:
    RoomHandler*     roomHandler;
    GameScene*       currScene;
    SceneHandler*    sceneHandler;
    AIHandler*       aiHandler;
    ResourceManager* resourceManager;
    UIRenderer*      uiRenderer;


    float nrOfEnemiesPerRoom = 0; // Set based on nr of Tile per room


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

    uint32_t fontTextureIndex;

    void spawnTank( const int tankIdx,  const glm::vec3& pos);
    void spawnLich( const int lichIdx,  const glm::vec3& pos);
    void spawnSwarmGroup(const int swarmStartIdx, const glm::vec3& pos); //TODO: Do we need to have a vector of pos; say 2 to let a swarm spawn over two tiles?
    void spawnSwarm(const int swarmIdx, const glm::vec3& pos);

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

    
};