#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"

GameScene::GameScene() :
	camEntity(-1), entity(-1)
{
    roomPieces = std::vector<int>();
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
	camTransform.position = glm::vec3(0);
    

    roomHandler.generateRoom();

	    //TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER
    std::cout << " -----------------\n";
    for (int i = 0; i < 10; i++) {
        std::cout << "|";
        for (int j = 0; j < 10; j++) {
            std::cout << roomHandler.getRoomTile(i * 10 + j) << " ";
        }
        std::cout << "|\n";
    }

    std::cout << " -----------------\n";

    //add tile enities
	for (int i = 0; i < roomHandler.getNrTiles(); i++) 
	{
        int pieceID = this->createEntity();
        this->setComponent<MeshComponent>(pieceID);
        MeshComponent& mesh = this->getComponent<MeshComponent>(pieceID);

        std::string path = "room_piece_" + std::to_string(roomHandler.getTile(i).type) + ".obj";
        memcpy(mesh.filePath, path.c_str(), sizeof(mesh.filePath));

        Transform& transform = this->getComponent<Transform>(pieceID);
        transform.position = glm::vec3(roomHandler.getTile(i).position.x, roomHandler.getTile(i).position.y, 4.f);
        transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
        transform.scale = glm::vec3(0.04f);

        roomPieces.push_back(pieceID);
	}
}

void GameScene::update()
{
}
