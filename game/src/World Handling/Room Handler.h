#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;

#define REAL_LAYOUT 1

class RoomHandler
{
public:
	static const float OUT_OF_BOUNDS;
	static const float ROOM_WIDTH;

private:

	RoomGenerator generator;
	RoomLayout roomLayout;
	Scene* scene;

	int activeRoomIdx;
	float roomWidth;

	float roomGridSize;

	// Temp
	struct RoomStorage
	{
		std::vector<int> entities;
		std::vector<glm::vec3> startPositions;
	};

	std::vector<RoomStorage> rooms;

	// return type value temp (should be void)
	int setActiveRoom(int index);

#ifdef _DEBUG
	void reload();
#endif

public:
	RoomHandler();

	void init(Scene* scene, int roomSize, int tileTypes);

	void generate();
	void update();
};