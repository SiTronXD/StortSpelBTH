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
    Entity portal;

    int numRoomsCleared;
    bool newRoomFrame;

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
    virtual void onTriggerStay(Entity e1, Entity e2) override;

private:
    void aiExample();
    bool allDead();
	
	void createPortal();
};
