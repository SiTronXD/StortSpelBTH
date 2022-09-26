#pragma once

#include "vengine.h"

class GameScene : public Scene
{
private:
	int camEntity;
	int entity;
	std::vector<int> roomPieces;

	int ROOM_SIZE = 9; //TODO: get from config
	int HALF_ROOM = ROOM_SIZE / 2;

	int getIndexFromVec2(int x, int y);
	void generateRoom();
	void placeRoomPiece(glm::vec2 position, int room[], int depth);
	void placeTile(int tileType, glm::vec2 gridPosition, glm::vec2 worldPosition, int room[]);
	glm::vec2 getFreeLarge(glm::vec2 position, int room[]);
	glm::vec2 getFreeAdjacent(glm::vec2 position, glm::vec2 dir, int room[]);

public:
	GameScene();
	virtual ~GameScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

