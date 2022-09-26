#include "GameScene.h"

//#include "TestSystem.h"
#include "MovementSystem.hpp"
#include "CombatSystem.hpp"
#include "Room.h"

GameScene::GameScene() :
	camEntity(-1), entity(-1)
{
	this->createSystem<MovementSystem>(this);
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	std::cout << "Test scene init" << std::endl;

	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity, 1.0f);
	this->setMainCamera(this->camEntity);
	Transform& camTransform = this->getComponent<Transform>(this->camEntity);
	camTransform.position = glm::vec3(1.0f);

	this->entity = this->createEntity();
	this->setComponent<MeshComponent>(this->entity);
	this->setComponent<Movement>(this->entity);
	this->setComponent<Combat>(this->entity);
	this->createSystem<CombatSystem>(this, entity);
	Transform& transform = this->getComponent<Transform>(this->entity);
	transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(5.0f);

	Room room;
	room.generateRoom();

}


void GameScene::update()
{
}
