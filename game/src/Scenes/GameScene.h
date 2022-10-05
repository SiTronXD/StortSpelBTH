#pragma once

#include "vengine.h"
#include "../World Handling/RoomHandler.h"
#include "../World Handling/Room.hpp"

class GameScene: public Scene
{
private:
    int camEntity;
    int entity;
    std::vector<int> roomPieces;

    RoomHandler roomHandler;


    //Room test variables
	int boss;
	std::vector<int> rooms;
	int doors[4];
	bool foundBoss;
	int bossHealth;
	int roomID;

    //Room stuff
	void setRoomVar();
	void setUpRooms();
	void runRoomIteration();

public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};
