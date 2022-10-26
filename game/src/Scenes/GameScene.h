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
    std::vector<int> swarmEnemies;
    std::vector<SwarmGroup*> swarmGroups;
    int camEntity;
	int entity;

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;

private:
    void aiEaxample();

};
