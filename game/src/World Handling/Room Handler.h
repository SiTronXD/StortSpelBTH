#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;

class RoomHandler
{
private:
	RoomGenerator generator;
	RoomLayout roomLayout;
	Scene* scene;

	// Temp
	struct RoomStorage
	{
		std::vector<int> entities;
	};

	std::vector<RoomStorage> rooms;

	void generate();

public:
	RoomHandler(Scene* scene, int roomSize, int tileTypes);

	void update();
};