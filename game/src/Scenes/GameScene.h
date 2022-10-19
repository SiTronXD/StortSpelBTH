#pragma once

#include "vengine.h"
#include "../World Handling/RoomHandler.h"
#include "../World Handling/Room.hpp"

class GameScene: public Scene
{
private:
    int camEntity;
    int entity;

    //Room test variables
	int boss;
	int doors[4];
	bool foundBoss;
	int bossHealth;
	int roomID;


public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};
