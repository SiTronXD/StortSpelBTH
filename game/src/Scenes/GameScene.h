#pragma once

#include "vengine.h"
#include "../World Handling/Room Handler.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"

class GameScene: public Scene
{
private:

    RoomHandler roomHandler;
    AIHandler* aiHandler = nullptr;

    Entity playerID;
    Entity floor;
    std::vector<int> enemyIDs;
    std::vector<SwarmGroup*> swarmGroups;

    uint32_t hpBarBackgroundTextureID;
    uint32_t hpBarTextureID;

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;

private:
    void aiExample();

};
