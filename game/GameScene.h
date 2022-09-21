#pragma once

#include "vengine.h"

class GameScene : public Scene
{
private:
	int camEntity;
	int entity;
	std::vector<int> roomPieces;

	void generateRoom();
	void placeRoomPiece(glm::vec2 position, std::vector<glm::vec2> &placedPieces, int depth);

public:
	GameScene();
	virtual ~GameScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

