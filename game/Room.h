#pragma once

#include "vengine.h"
#include <random>

class Room
{
private:
	int TILE_TYPES = 4; //TODO: get from config
	int ROOM_SIZE = 9; //TODO: get from config
	int HALF_ROOM = ROOM_SIZE / 2;

	int* room;

	int getIndexFromVec2(int x, int y) { return y * ROOM_SIZE + x; }

	void addPiece(glm::vec2 position, int depth);
	void placeTile(int tileType, glm::vec2 gridPosition, glm::vec2 worldPosition);
	glm::vec2 getFreeLarge(glm::vec2 position);
	glm::vec2 getFreeAdjacent(glm::vec2 position, glm::vec2 dir);

public:
	void generateRoom();
};

