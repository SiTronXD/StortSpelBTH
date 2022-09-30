#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

GameScene::GameScene():
	camEntity(-1), entity(-1)
{
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
	//this->createSystem<CombatSystem>(this, entity);
	//this->createSystem<MovementSystem>(this, entity);

	int floor = this->createEntity();
    this->setComponent<MeshComponent>(floor);
    Transform& transform2 = this->getComponent<Transform>(floor);
    transform2.position   = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale      = glm::vec3(100.f, 0.1f, 100.f);

	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity, 1.0f);
	this->setComponent<CameraMovement>(this->camEntity);
	this->setMainCamera(this->camEntity);
	Transform& camTransform = this->getComponent<Transform>(this->camEntity);
	camTransform.position = glm::vec3(1.0f);

    this->createSystem<CameraMovementSystem>(this, this->entity);
}

void GameScene::update()
{
}
