#pragma once

#include "vengine.h"

class GameScene : public Scene
{
private:
	int camEntity;
	int entity;
	int anotherEntity;
public:
	GameScene();
	virtual ~GameScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

