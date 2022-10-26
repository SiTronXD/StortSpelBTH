#pragma once

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class GameScene: public Scene
{
private:

    RoomHandler roomHandler;

    Entity playerID;
    std::vector<Entity> enemyIDs;
    int swarm;

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};
