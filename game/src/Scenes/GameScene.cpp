#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

// decreaseFps used for testing game with different framerates
double decreaseFps(double value);
double heavyFunction(double value);

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
	this->createSystem<CombatSystem>(this, entity);
	this->createSystem<MovementSystem>(this, this->entity);

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

	static double value = 1234567890.0;
	value = decreaseFps(value);
}

double decreaseFps(double value)
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
		result /= std::sqrt(heavyFunction(result / value));
		result /= std::sqrt(heavyFunction(result / value));
		result /= std::sqrt(heavyFunction(result / value));
		result /= std::sqrt(heavyFunction(result / value));
		result /= std::sqrt(heavyFunction(result / value));
    }

	return result;
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