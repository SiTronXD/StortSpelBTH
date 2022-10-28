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
	int swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

	roomHandler.init(this, this->getResourceManager(), this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));
	roomHandler.generate();
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

	uint32_t swordId = this->getResourceManager()->addMesh("assets/models/Sword.obj");
	
	Entity sword = this->createEntity();
	this->setComponent<MeshComponent>(sword);
	this->getComponent<MeshComponent>(sword).meshID = swordId;
}

void GameScene::update()
{
	if (Input::isKeyPressed(Keys::E)) 
	{
		// Call when a room is cleared
		roomHandler.roomCompleted();		
	}

	// Player entered a new room
	if (roomHandler.checkPlayer(this->getComponent<Transform>(playerID).position))
	{
		const std::vector<Entity>& entites = roomHandler.getFreeTiles();
		for (Entity entity : entites)
		{
			// Hi
		}
	}

	/*if (this->hasComponents<Collider, Rigidbody>(this->playerID))
	{
		Rigidbody& rb = this->getComponent<Rigidbody>(this->playerID);
		glm::vec3 vec = glm::vec3(Input::isKeyDown(Keys::LEFT) - Input::isKeyDown(Keys::RIGHT), 0.0f, Input::isKeyDown(Keys::UP) - Input::isKeyDown(Keys::DOWN));
		float y = rb.velocity.y;
		rb.velocity = vec * 10.0f;
		rb.velocity.y = y + Input::isKeyPressed(Keys::SPACE) * 5.0f;
	}*/

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