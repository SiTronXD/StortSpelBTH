#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"

GameScene::GameScene() :
	camEntity(-1), entity(-1)
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

    roomHandler.generateRoom();

	    //TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER
    std::cout << " -----------------\n";
    for (int i = 0; i < 9; i++) {
        std::cout << "|";
        for (int j = 0; j < 9; j++) {
            std::cout << roomHandler.getRoomTile(i * 9 + j) << " ";
        }
        std::cout << "|\n";
    }

    std::cout << " -----------------\n";

	this->entity = this->createEntity();
	this->setComponent<MeshComponent>(this->entity);
    MeshComponent& mesh = this->getComponent<MeshComponent>(this->entity);
    std::string path = "room_piece_";
    path += std::to_string(roomHandler.getRoomTile(40));
    path += ".obj";
    memcpy(mesh.filePath, path.c_str(), sizeof(mesh.filePath));
            
	this->setComponent<Movement>(this->entity);
	this->createSystem<MovementSystem>(this, entity);
	this->setComponent<Combat>(this->entity);
	this->createSystem<CombatSystem>(this, entity);
	Transform& transform = this->getComponent<Transform>(this->entity);
	transform.position = glm::vec3(0.0f, 0.0f, 100.0f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	//transform.scale = glm::vec3(5.0f);


}

void GameScene::update()
{
}
