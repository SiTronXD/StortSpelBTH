#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

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
	int player = this->createEntity();
	this->setComponent<MeshComponent>(player);
	this->setComponent<Movement>(player);
	this->getComponent<Transform>(player).rotation.x = -90.f;
	this->getComponent<Transform>(player).position.y = 2.5f;
	this->createSystem<MovementSystem>(this, player);

	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity, 1.0f);
	this->setMainCamera(this->camEntity);
    this->createSystem<CameraMovementSystem>(this, player);

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

	const float TILE_SCALE = 25.f;

	for (int i = 0; i < roomHandler.getNrTiles(); i++) 
	{
        int pieceID = this->createEntity();
        this->setComponent<MeshComponent>(pieceID);
        MeshComponent& mesh = this->getComponent<MeshComponent>(pieceID);

        std::string path = "room_piece_" + std::to_string(roomHandler.getTile(i).type) + ".obj";
        memcpy(mesh.filePath, path.c_str(), sizeof(mesh.filePath));

        Transform& transform = this->getComponent<Transform>(pieceID);
        transform.position = glm::vec3(roomHandler.getTile(i).position.x * TILE_SCALE, 0.f, roomHandler.getTile(i).position.y * TILE_SCALE);

        roomPieces.push_back(pieceID);
	}
}

void GameScene::update()
{
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