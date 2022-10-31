#include "GameOverScene.h"
#include "TutorialScene.h"

void GameOverScene::init() {}

void GameOverScene::start() {}

void GameOverScene::update()
{
	// Switch scene
	if (Input::isKeyPressed(Keys::SPACE))
	{
		this->switchScene(new TutorialScene(), "scripts/TutorialScene.lua");
	}
}
