#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

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
	
	this->setComponent<Collider>(playerID, Collider::createBox(glm::vec3(2.f)));

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
	//if (roomHandler.newRoom(this->getComponent<Transform>(playerID).position))
	//{
	//	const std::vector<Entity>& entites = roomHandler.getFreeTiles();
	//	for (Entity entity : entites)
	//	{
	//		// Hi
	//	}
	//}
	
#ifdef _CONSOLE

	static bool renderDebug = false;
	if (ImGui::Begin("Debug"))
	{
		if (ImGui::Checkbox("Render debug shapes", &renderDebug))
		{
			this->getPhysicsEngine()->renderDebugShapes(renderDebug);
		}
		const glm::vec3& playerPos = this->getComponent<Transform>(playerID).position;
		ImGui::Text("Player pos: (%d, %d, %d)", (int)playerPos.x, (int)playerPos.y, (int)playerPos.z);
		ImGui::Separator();
	}
	ImGui::End();

	roomHandler.imgui();

	decreaseFps();
#endif
}

void GameScene::onTriggerStay(Entity e1, Entity e2)
{
	Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;
	printf("trig stay\n");
	if (player == playerID)
	{
		printf("Player stay\n");
		Entity other = e1 == player ? e2 : e1;

		// player triggered a trigger :]
		Transform& playerTra = this->getComponent<Transform>(playerID);
		
		if (roomHandler.newRoom(playerTra.position, other))
		{
			printf("Hello?\n");
		}
	}
}

#ifdef _CONSOLE
void decreaseFps()
{
	static double result = 1234567890.0;

	static int num = 0;
	if (ImGui::Begin("Debug"))
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
#endif