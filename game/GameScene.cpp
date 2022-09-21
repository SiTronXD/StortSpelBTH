#include "GameScene.h"

//#include "TestSystem.h"
#include "MovementSystem.hpp"
#include "CombatSystem.hpp"

#include <random>

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

	generateRoom();
}

void GameScene::generateRoom()
{
	std::vector<glm::vec2> placedPieces = std::vector<glm::vec2>();

	//generate first room at 0,0 and depth 0
	placeRoomPiece(glm::vec2(0, 0), placedPieces, 0);



	//TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER
	std::cout << " -----------------\n";
	for (int i = -4; i <= 4; i++)
	{
		std::cout << "|";
		for (int j = -4; j <= 4; j++)
		{
			if(j == 0 && i == 0)
				std::cout << "  ";
			else if (std::find(placedPieces.begin(), placedPieces.end(), glm::vec2(j, i)) != placedPieces.end())
				std::cout << "  ";
			else
				std::cout << "X ";
		}
		std::cout << "|\n";
	}

	std::cout << " -----------------\n";
	
}

//recursive function to place room pieces around the first piecce
void GameScene::placeRoomPiece(glm::vec2 position, std::vector<glm::vec2> &placedPieces, int depth)
{
	//add piece only if tile is within max range of center
	if (abs(position.x) <= 4 && abs(position.y) <= 4)
	{
		std::random_device rd;	//obtain random number from hardware
		std::mt19937 gen(rd());	//seed generator
		if (std::find(placedPieces.begin(), placedPieces.end(), position) == placedPieces.end())
		{
			std::uniform_int_distribution<> tileTypeRange(1, 1);
			int tileType = tileTypeRange(gen);

			int pieceID = this->createEntity();
			//this->setComponent<MeshComponent>(pieceID);
			Transform& transform = this->getComponent<Transform>(pieceID);

			switch (tileType)
			{
			case 1:
				transform.position.x = position.x;
				transform.position.z = position.y;

				this->roomPieces.push_back(pieceID);
				placedPieces.push_back(position);
				break;
			case 2:

				break;
			case 3:
				break;
			case 4:
				break;
			}
		}
		
		std::uniform_int_distribution<> distr(0, depth * 3);	//smaller chance of creating new piece the deeper we are in the tree

		//GENERATE PIECE UP
		int newPiece = (distr(gen));
		glm::vec2 nextPos = position + glm::vec2(0, 1);
		if (newPiece <= 1)
		{
			placeRoomPiece(nextPos, placedPieces, depth + 1);
		}

		//GENERATE PIECE RIGHT
		newPiece = (distr(gen));
		nextPos = position + glm::vec2(1, 0);
		if (newPiece <= 1)
		{
			placeRoomPiece(nextPos, placedPieces, depth + 1);
		}

		//GENERATE PIECE DOWN
		newPiece = (distr(gen));
		nextPos = position + glm::vec2(0, -1);
		if (newPiece <= 1)
		{
			placeRoomPiece(nextPos, placedPieces, depth + 1);
		}

		//GENERATE PIECE LEFT
		newPiece = (distr(gen));
		nextPos = position + glm::vec2(-1, 0);
		if (newPiece <= 1)
		{
			placeRoomPiece(nextPos, placedPieces, depth + 1);
		}
	}
}

void GameScene::update()
{
}
