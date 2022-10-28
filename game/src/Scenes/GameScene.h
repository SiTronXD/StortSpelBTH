#pragma once

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class GameScene: public Scene
{
private:

    RoomHandler roomHandler;

    Entity playerID;
    Entity floor;
public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
    virtual void onTriggerStay(Entity e1, Entity e2) override;
};
