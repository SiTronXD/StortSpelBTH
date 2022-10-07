#pragma once
#include <vector>
#include "vengine/application/Scene.hpp"

#include "../Components/Room.h"

#define LOGICAL_LAYOUT 0
#define BRANCH 0

class RoomLayout
{
private:
	int boss;
	std::vector<int> rooms;
	int doors[4];
	bool foundBoss;
	int bossHealth;
	int roomID;

	Scene* scene;

	// Private Functions

	void initRooms(int roomID);
#if BRANCH == 1
	bool setRandomBranch(int numRooms);
	void setBranch(int index, bool left, int size);
#endif
	bool setBoss(int numRooms);
	bool setExit();
	bool setShortcut(int numBranches, int numRooms);
	
	int getEndWithRightAvaliable();
	int getEndWithLeftAvaliable();
	
	
	void placeDoors(int& roomID);
	bool canGoForward();
	bool canGoBack();
	bool canGoLeft();
	bool canGoRight();

	// Prints
	void fightBoss(int& boss, int& bossHealth, int& roomID, bool& foundBoss);
	void printDoorOptions(int& roomID);
	void traverseRoomsConsole();
	
	// Unused
	int numEnds();

	// Statics
	static std::string typeToString(Room::ROOM_TYPE type);

public:
	RoomLayout(Scene* scene);
	virtual ~RoomLayout();

	int getRoomID(int index)
	{
		return rooms[index];
	}
	int getNumRooms() const
	{
		return (int)rooms.size();
	}

	int setUpRooms();
	bool traverseRooms(int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta);


};