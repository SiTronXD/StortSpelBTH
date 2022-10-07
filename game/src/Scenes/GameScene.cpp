#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

GameScene::GameScene():
	camEntity(-1), entity(-1)
{
    roomPieces = std::vector<int>();
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	this->entity = this->createEntity();
	this->setComponent<MeshComponent>(this->entity);
	this->setComponent<Movement>(this->entity);
	this->setComponent<Combat>(this->entity);
	Transform& transform = this->getComponent<Transform>(this->entity);
	transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(5.0f);
	this->createSystem<CombatSystem>(this, entity);
	this->createSystem<MovementSystem>(this, this->entity);

	int floor = this->createEntity();
    this->setComponent<MeshComponent>(floor);
    Transform& transform2 = this->getComponent<Transform>(floor);
    transform2.position   = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale      = glm::vec3(100.f, 0.1f, 100.f);

	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity, 1.0f);
	this->setMainCamera(this->camEntity);
    this->createSystem<CameraMovementSystem>(this, player);

    this->createSystem<CameraMovementSystem>(this, this->entity);
}

void GameScene::update()
{
	decreaseFps();
}

void GameScene::setRoomVar()
{
	foundBoss = false;
	bossHealth = 100;
	roomID = 0;
}

void GameScene::setUpRooms()
{
	
	boss = this->createEntity();
	this->setComponent<MeshComponent>(boss);
	this->getComponent<Transform>(boss).position = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
	for (int i = 0; i < 4; i++)
	{
		doors[i] = this->createEntity();
		this->setComponent<MeshComponent>(doors[i]);
	}
	initRooms(*this, rooms, doors, roomID);
	std::cout << "Num rooms: " << rooms.size() << std::endl;
	std::cout << "Slow: WASD" << std::endl << "Fast: HBNM" << std::endl;
}

void GameScene::runRoomIteration()
{
	if (traverseRooms(*this, rooms, doors, roomID, boss, bossHealth, foundBoss, Time::getDT())) {
		std::cout << "You found the exit!\n";
		Transform& bossTransform = this->getComponent<Transform>((boss));
		bossTransform.position = this->getComponent<Transform>(rooms[roomID]).position + glm::vec3(0.0f, 10.0f, 20.0f);
		bossTransform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
		bossTransform.rotation = glm::vec3(bossTransform.rotation.x + (Time::getDT() * 50), bossTransform.rotation.y + (Time::getDT() * 50), bossTransform.rotation.z + (Time::getDT() * 50));
	}
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
