#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

GameScene::GameScene() : playerID(-1)
{
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	int ghost = this->getResourceManager()->addMesh("assets/models/ghost.obj");

	roomHandler.init(this, this->getResourceManager(), this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));
	roomHandler.generate();
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

	this->setComponent<Collider>(playerID, Collider::createCapsule(2.f, 2.f));
}

void GameScene::update()
{
	Transform& playerTra = this->getComponent<Transform>(playerID);
	Collider& playerCol = this->getComponent<Collider>(playerID);

	this->getDebugRenderer()->renderCapsule(
		playerTra.position, playerTra.rotation, playerCol.height, playerCol.radius, glm::vec3(0.f, 1.f, 1.f));

	roomHandler.update(playerTra.position);
	roomHandler.drawColliders(this->getSceneHandler());

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