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
private:
    RoomHandler*     roomHandler;
    GameScene*       currScene;
    SceneHandler*    sceneHandler;
    AIHandler*       aiHandler;
    ResourceManager* resourceManager;
    UIRenderer*      uiRenderer;

    int nrOfTanks  = 0;
    int nrOfLichs  = 0;
    int nrOfSwarms = 2;

    int numOfGroups = 4;
    int group_size = 6;

    std::vector<int> allEntityIDs;
    std::vector<int> swarmIDs;
    std::vector<int> lichIDs;
    std::vector<int> tankIDs;
    std::vector<SwarmGroup*> swarmGroups;

    uint32_t fontTextureIndex;

    void spawnTank( const int tankIdx,  const glm::vec3& pos);
    void spawnLich( const int lichIdx,  const glm::vec3& pos);
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

    bool allDead();

    
};