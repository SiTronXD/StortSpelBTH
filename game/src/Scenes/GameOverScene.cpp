#include "GameOverScene.h"
#include "MainMenu.h"

void GameOverScene::init() {}

void GameOverScene::start() {}

void GameOverScene::update()
{
	// Switch scene
	if (Input::isKeyPressed(Keys::SPACE))
	{
		this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
	}
}
