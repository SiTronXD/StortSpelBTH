#pragma once
#include <vector>
#include <string>
#include "../Components/Room.h"

#define RANDOM_POSITION 0
#define PLAY 0

class Scene;

// MOVE DOORS TO ROOM HANDLER
// MOVE DOORS TO ROOM HANDLER
// MOVE DOORS TO ROOM HANDLER

class RoomLayout
{
private:
	int boss;
	std::vector<int> rooms;
	bool foundBoss;
	int roomID;
	int numMainRooms;

	Scene* scene;

#if !RANDOM_POSITION
	float distance;
#endif

	// Private Functions
	void setUpRooms(int numRooms);

	bool setRandomBranch(int numRooms);
	void setBranch(int index, bool left, int size);

	bool setBoss(int numRooms);
	bool setExit();
	bool setShortcut(int numBranches, int numRooms);
	
	int getEndWithRightAvaliable();
	int getEndWithLeftAvaliable();
	
	
#if PLAY
	bool canGoForward();
	bool canGoBack();
	bool canGoLeft();
	bool canGoRight();

	void fightBoss(int& boss, int& bossHealth, int& roomID, bool& foundBoss);
	void printDoorOptions(int& roomID);
	void traverseRoomsConsole();

	// Statics
	static std::string typeToString(Room::ROOM_TYPE type);
#endif

public:
	RoomLayout();
	virtual ~RoomLayout();

	void init(Scene* scene, float distance);

	void generate();
	void clear();

	int getRoomID(int index)
	{
		return rooms[index];
	}
	int getNumRooms() const
	{
		return (int)rooms.size();
	}
	int getNumMainRooms() const
	{
		return numMainRooms;
	}

	void createDoors(int roomID, const glm::vec2* positions);

	bool traverseRooms(int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta);
};