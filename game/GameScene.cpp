#include "GameScene.h"

//#include "TestSystem.h"
#include "MovementSystem.hpp"
#include "CombatSystem.hpp"

GameScene::GameScene():
	camEntity(-1), entity(-1)
{
	this->createSystem<MovementSystem>(this);
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	//this->createSystem<TestSystem>();

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
}

void GameScene::update()
{
	//glm::vec3 move = glm::vec3(
	//	Input::isKeyDown(Keys::D) - Input::isKeyDown(Keys::A), 
	//	Input::isKeyDown(Keys::Q) - Input::isKeyDown(Keys::E),
	//	Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
	//Transform& transform = this->getComponent<Transform>(this->enitity);

	//transform.position += (move.x * transform.right() + move.y * transform.forward() + move.z * transform.up()) * 25.0f * Time::getDT();
	//transform.rotation.z += 10.0f * Time::getDT();
}
