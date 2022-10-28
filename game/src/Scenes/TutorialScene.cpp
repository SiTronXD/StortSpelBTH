#include "TutorialScene.h"
#include "GameScene.h"

void TutorialScene::init() 
{
	this->shouldLoadNextScene = false;
}

void TutorialScene::start() 
{

}

void TutorialScene::update() 
{
	// Actually switch next frame
	if (this->shouldLoadNextScene)
	{
		this->switchScene(new GameScene(), "scripts/GameScene.lua");
	}

	// Check if the scene should be switched
	if (Input::isKeyPressed(Keys::SPACE))
	{
		this->shouldLoadNextScene = true;
	}
}
