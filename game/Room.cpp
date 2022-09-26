#include "Room.h"
#include <iostream>//TODO: ONLY FOR VISUALIZING ROOM LAYOUT. REMOVE LATER


void Room::generateRoom()
{
	//create 2d array representing room and set all room pieces to 0
	room = new int[ROOM_SIZE * ROOM_SIZE];

	memset(room, 0, sizeof(int) * ROOM_SIZE * ROOM_SIZE);
	//generate first room piece in middle (4,4) and depth 0
	addPiece(glm::vec2(HALF_ROOM, HALF_ROOM), 0);

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

void Room::addPiece(glm::vec2 position, int depth)
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
				placeTile(tileType, position, position);
				break;
			case 2:	//1x2 piece, need to check if adjecent tile is free
			{
				int dY = getFreeAdjacent(position, glm::vec2(0, 1)).y;
				if (dY != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0, 0.5 * dY));
					room[getIndexFromVec2(x, y + dY)] = tileType;
					break;
				}
			}
			case 3: //2x1
			{
				int dX = getFreeAdjacent(position, glm::vec2(1, 0)).x;
				if (dX != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0.5 * dX, 0));
					room[getIndexFromVec2(x + dX, y)] = tileType;
					break;
				}
			}
			case 4:	//2x2
			{
				glm::vec2 dir = getFreeLarge(position);
				if (dir.x != 0)
				{
					placeTile(tileType, position, position + glm::vec2(0.5 * dir.x, 0.5 * dir.y));
					room[getIndexFromVec2(x + dir.x, y)] = tileType;
					room[getIndexFromVec2(x, y + dir.y)] = tileType;
					room[getIndexFromVec2(x + dir.x, y + dir.y)] = tileType;
					break;
				}
			}
			default:
				placeTile(1, position, position);
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
				addPiece(nextPos, depth + 1);
			}
		}
	}
}

void Room::placeTile(int tileType, glm::vec2 gridPosition, glm::vec2 worldPosition)
{
	room[getIndexFromVec2(gridPosition.x, gridPosition.y)] = tileType;
}

glm::vec2 Room::getFreeLarge(glm::vec2 position)
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

glm::vec2 Room::getFreeAdjacent(glm::vec2 position, glm::vec2 dir)
{
	if (room[getIndexFromVec2(position.x + dir.x, position.y + dir.y)] < 1)
		return dir;
	dir *= -1;
	if (room[getIndexFromVec2(position.x + dir.x, position.y + dir.y)] < 1)
		return dir;
	return glm::vec2(0);
}