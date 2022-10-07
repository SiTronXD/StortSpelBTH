#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

GameScene::GameScene()
{

}

GameScene::~GameScene()
{
}

void GameScene::init()
{
    roomHandler.init(this, this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));

	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity, 1.0f);
	this->setMainCamera(camEntity);

	int playerID = this->createEntity();
	this->setComponent<MeshComponent>(playerID);
	this->setComponent<Movement>(playerID);
	this->getComponent<Transform>(playerID).rotation.x = -90.f;
	this->getComponent<Transform>(playerID).position.y = 2.5f;
	this->createSystem<MovementSystem>(this, playerID);
    this->createSystem<CameraMovementSystem>(this, playerID);
	roomHandler.generate();
}

void GameScene::update()
{
	roomHandler.update();

	// Temp
	if (Input::isKeyPressed(Keys::R))
	{
		// Bröken kröken fröken jöken göken löken söken möten
		//roomHandler.reload();
	}

	decreaseFps();
}

void decreaseFps()
{
	static double result = 1234567890.0;

	static int num = 0;
    if (ImGui::Begin("FPS decrease")) 
	{
		ImGui::Text("Fps %f", 1.f / Time::getDT());
        ImGui::InputInt("Loops", &num);
    }
    ImGui::End(); 

    for (int i = 0; i < num; i++) 
	{
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
    }
}

double heavyFunction(double value)
{
	double result = 1234567890.0;
	for (size_t i = 0; i < 3000; i++)
	{
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 0.892375892))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 1.476352734))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 2.248923885))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.691284908))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.376598278))));
	}
	
	return result;
}
