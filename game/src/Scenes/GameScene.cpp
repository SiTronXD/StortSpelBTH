#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

GameScene::GameScene() : playerID(-1), swarm(-1)
{
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
	for (size_t i = 0; i < 10; i++)
	{
		this->enemyIDs.emplace_back();
		this->enemyIDs.back() = this->createEntity();
		this->setComponent<MeshComponent>(this->enemyIDs.back(), this->swarm);
		this->setComponent<AiMovement>(this->enemyIDs.back());
		this->setComponent<AiCombat>(this->enemyIDs.back());
		this->setInactive(this->enemyIDs.back());
	}

	roomHandler.init(this, this->getResourceManager(), this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));
	roomHandler.generate();
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

	this->setComponent<Combat>(this->playerID);
	this->createSystem<AiMovementSystem>(this->getSceneHandler(), this->playerID);
	this->createSystem<AiCombatSystem>(this->getSceneHandler(), this->playerID);
}

void GameScene::update()
{
	if (Input::isKeyPressed(Keys::E))
	{
		// Call when a room is cleared
		roomHandler.roomCompleted();
		for (size_t i = 0; i < this->enemyIDs.size(); i++)
		{
			this->setInactive(this->enemyIDs[i]);
		}
	}

	// Player entered a new room
	if (roomHandler.checkPlayer(this->getComponent<Transform>(playerID).position))
	{
		int idx = 0;
		const std::vector<Entity>& entites = roomHandler.getFreeTiles();
		for (Entity entity : entites)
		{
			if (idx != 10)
			{
				this->setActive(this->enemyIDs[idx]);
				Transform& transform = this->getComponent<Transform>(this->enemyIDs[idx]);
				Transform& tileTrans = this->getComponent<Transform>(entity);
				float tileWidth = rand() % (int)RoomHandler::TILE_WIDTH + 0.01f;
				transform.position = tileTrans.position;
				transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);
				idx++;
			}
		}
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