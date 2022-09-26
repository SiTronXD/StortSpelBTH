#include "GameScene.h"

//#include "TestSystem.h"
#include "MovementSystem.hpp"
#include "CombatSystem.hpp"

#include <random>

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

	generateRoom();
}

void GameScene::generateRoom()
{
	//create 2d array representing room and set all room pieces to 0
	int* room = new int[ROOM_SIZE * ROOM_SIZE];

	memset(room, 0, sizeof(int) * ROOM_SIZE * ROOM_SIZE);
	//generate first room piece in middle (4,4) and depth 0
	placeRoomPiece(glm::vec2(4, 4), room, 0);



	//TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER
	std::cout << " -----------------\n";
	for (int i = 0; i < ROOM_SIZE; i++)
	{
		std::cout << "|";
		for (int j = 0; j < ROOM_SIZE; j++)
		{
			std::cout << room[i * ROOM_SIZE + j] << " ";
		}
		std::cout << "|\n";
	}

	std::cout << " -----------------\n";

}

int GameScene::getIndexFromVec2(int x, int y)
{
	return y * ROOM_SIZE + x;
}

//recursive function to place room pieces around the first piecce
void GameScene::placeRoomPiece(glm::vec2 position, int room[], int depth)
{
	int x = position.x;
	int y = position.y;
	int index = getIndexFromVec2(x, y);

	//add piece only if tile is within room bounds
	if (abs(x - HALF_ROOM) < HALF_ROOM && abs(y - HALF_ROOM) < HALF_ROOM)
	{
		std::random_device rd;	//obtain random number from hardware
		std::mt19937 gen(rd());	//seed generator
		if (room[index] < 1)
		{
			std::uniform_int_distribution<> tileTypeRange(1, 4);	//TODO: change to nr tile types
			int tileType = tileTypeRange(gen);

			//TODO: clean up code
			switch (tileType)
			{
			case 1: //Simple 1x1 piece
				placeTile(tileType, position, position, room);
				break;
			case 2:	//1x2 piece, need to check if adjecent tile is free
			{
				int dY = getFreeAdjacent(position, glm::vec2(0, 1), room).y;
				if (dY != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0, 0.5 * dY), room);
					room[getIndexFromVec2(x, y + dY)] = tileType;
					break;
				}
			}
			case 3: //2x1
			{
				int dX = getFreeAdjacent(position, glm::vec2(1, 0), room).x;
				if (dX != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0.5 * dX, 0), room);
					room[getIndexFromVec2(x + dX, y)] = tileType;
					break;
				}
			}
			case 4:	//2x2
			{
				glm::vec2 dir = getFreeLarge(position, room);
				if (dir.x != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0.5 * dir.x, 0.5 * dir.y), room);
					room[getIndexFromVec2(x + dir.x, y)] = tileType;
					room[getIndexFromVec2(x, y + dir.y)] = tileType;
					room[getIndexFromVec2(x + dir.x, y + dir.y)] = tileType;
					break;
				}
			}
			default:
				placeTile(1, position, position, room);
				break;
			}
		}

		std::uniform_int_distribution<> distr(0, depth * 3);	//smaller chance of creating new piece the deeper we are in the tree
		//check if should place tiles above, below, right and left 
		for (int i = 0; i < 4; i++)
		{
			//use sin and cos to check adjacent tiles in clockwise order
			int dirX = sin(i * M_PI / 2);
			int dirY = cos(i * M_PI / 2);

			int newPiece = (distr(gen));
			glm::vec2 nextPos = position + glm::vec2(dirX, dirY);
			if (newPiece <= 1)
			{
				placeRoomPiece(nextPos, room, depth + 1);
			}
		}
	}
}

void GameScene::placeTile(int tileType, glm::vec2 gridPosition, glm::vec2 worldPosition, int room[])
{
	int pieceID = this->createEntity();
	//this->setComponent<MeshComponent>(pieceID);
	Transform& transform = this->getComponent<Transform>(pieceID);

	transform.position.x = worldPosition.x;
	transform.position.z = worldPosition.y;
	this->roomPieces.push_back(pieceID);
	room[getIndexFromVec2(gridPosition.x, gridPosition.y)] = tileType;
}

glm::vec2 GameScene::getFreeLarge(glm::vec2 position, int room[])
{
	int x = position.x;
	int y = position.y;

	int dY = 1;
	if (room[getIndexFromVec2(x, y + dY)] < 1)	//first check adjacent tile in Y
	{
		int dX = 1;
		if (room[getIndexFromVec2(x + dX, y)] < 1 && room[getIndexFromVec2(x + dX, y + dY)] < 1)
			return glm::vec2(dX, dY);
		dX = -1;
		if (room[getIndexFromVec2(x + dX, y)] < 1 && room[getIndexFromVec2(x + dX, y + dY)] < 1)
			return glm::vec2(dX, dY);
	}
	dY = -1;
	if (room[getIndexFromVec2(x, y + dY)] < 1)
	{
		int dX = 1;
		if (room[getIndexFromVec2(x + dX, y)] < 1 && room[getIndexFromVec2(x + dX, y + dY)] < 1)
			return glm::vec2(dX, dY);
		dX = -1;
		if (room[getIndexFromVec2(x + dX, y)] < 1 && room[getIndexFromVec2(x + dX, y + dY)] < 1)
			return glm::vec2(dX, dY);
	}
	return glm::vec2(0);
}

glm::vec2 GameScene::getFreeAdjacent(glm::vec2 position, glm::vec2 dir, int room[])
{
	if (room[getIndexFromVec2(position.x + dir.x, position.y + dir.y)] < 1)
		return dir;
	dir *= -1;
	if (room[getIndexFromVec2(position.x + dir.x, position.y + dir.y)] < 1)
		return dir;
	return glm::vec2(0);
}

void GameScene::update()
{
}
