#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;

class RoomHandler
{
private:
	// temp
	const float OUT_OF_BOUNDS = 1000;

	RoomGenerator generator;
	RoomLayout roomLayout;
	Scene* scene;

	int activeRoomIdx;

	// Temp
	struct RoomStorage
	{
		std::vector<int> entities;
		std::vector<glm::vec3> startPositions;
	};

	std::vector<RoomStorage> rooms;

	// return type value temp (should be void)
	int setActiveRoom(int index);

public:
	RoomHandler();

	void init(Scene* scene, int roomSize, int tileTypes);

	void reload();
	void generate();
	void update();
};