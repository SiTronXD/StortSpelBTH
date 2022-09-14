#include "GameScene.h"

GameScene::GameScene():
	camEntity(-1), enitity(-1)
{
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity, 1.0f);
	this->setMainCamera(this->camEntity);
	Transform& camTransform = this->getComponent<Transform>(this->camEntity);
	camTransform.position = glm::vec3(1.0f);

	this->enitity = this->createEntity();
	this->setComponent<MeshComponent>(this->enitity);
	Transform& transform = this->getComponent<Transform>(this->enitity);
	transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(5.0f);
}

void GameScene::update()
{
	glm::vec3 move = glm::vec3(
		Input::isKeyDown(Keys::D) - Input::isKeyDown(Keys::A), 
		Input::isKeyDown(Keys::Q) - Input::isKeyDown(Keys::E),
		Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
	Transform& transform = this->getComponent<Transform>(this->enitity);

	transform.position += (move.x * transform.right() + move.y * transform.forward() + move.z * transform.up()) * 25.0f * Time::getDT();
	transform.rotation.z += 10.0f * Time::getDT();
}
