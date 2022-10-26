#pragma once

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class GameScene: public Scene
{
private:

    RoomHandler roomHandler;

    Entity playerID;
    int camEntity;
	int entity;

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};
