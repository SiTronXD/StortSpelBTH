#pragma once

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class GameScene: public Scene
{
private:

    RoomHandler roomHandler;
    int playerID;

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};
