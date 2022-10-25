#include "Room Layout.h"
#include "vengine/dev/Log.hpp"

#include <ctime>

RoomLayout::RoomLayout()
	:distance(0.f), numMainRooms(0)
{
	std::srand((unsigned)time(0));	
}

RoomLayout::~RoomLayout()
{
}

void RoomLayout::setRoomDistance(float distance)
{
	this->distance = distance;
}

void RoomLayout::clear()
{
	rooms.clear();
	connections.clear();
}

void RoomLayout::generate()
{
	numMainRooms = 7; //rand() % 3 + 3;
	int numBranches = 14;// rand() % numMainRooms + 1;

	connections.reserve(size_t(numMainRooms + numBranches));

	setUpRooms(numMainRooms);
	for (int i = 0; i < numBranches; i++)
	{
		if (!setRandomBranch(numMainRooms)) 
		{
			Log::warning("Failed creating branch");
		}
	}
}

void RoomLayout::setUpRooms(int numRooms)
{
	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numRooms; i++)
	{
		rooms.emplace_back();

		RoomData& curRoom = rooms[i];
		glm::vec3& curPosition = curRoom.position;

		curPosition.z = i * distance;

		//First room is always the start room
		if (i == 0)
		{
			curRoom.type = RoomData::Type::START_ROOM;
			curPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			if (i > 0)
			{
				rooms[size_t(i - 1)].up = i;
				rooms[i].down = i - 1;

				connections.emplace_back(i, i - 1);
			}

			//one in five to become a hard room
			if (rand() % 5 == 0)
			{
				curRoom.type = RoomData::Type::HARD_ROOM;
			}
			else
			{
				curRoom.type = RoomData::Type::NORMAL_ROOM;
			}
		}
	}
}

bool RoomLayout::setRandomBranch(int numRooms)
{
	int branchSize = 2;// rand() % 2 + 1;

	bool foundSpot = false;
	int spot = rand() % (numRooms - 1);
	int numTest = 0;

	if (rooms[spot].left != -1 && rooms[spot].right != -1)
	{
		//Keep looking for a spot to place branch
		while (!foundSpot)
		{
			if (rooms[spot].left == -1 || rooms[spot].right == -1)
			{
				foundSpot = true;
				break;
			}
			if (++spot >= numRooms)
			{
				spot = 0;
			}
			if (++numTest > numRooms) {
				return false;
			}
		}
	}
	if (rooms[spot].left == -1 && rooms[spot].right == -1)
	{
		if (rand() % 2 == 0)
		{
			setBranch(spot, true, branchSize);
		}
		else
		{
			setBranch(spot, false, branchSize);
		}
	}
	else if (rooms[spot].left == -1)
	{
		setBranch(spot, true, branchSize);
	}
	else
	{
		setBranch(spot, false, branchSize);
	}

	return true;
}

void RoomLayout::setBranch(int index, bool left, int size)
{
	RoomData::Type roomType = RoomData::Type::NORMAL_ROOM;

	glm::vec3 position = rooms[index].position;

	if (rand() % 5 == 0)
	{
		roomType = RoomData::Type::HARD_ROOM;
	}
	else
	{
		roomType = RoomData::Type::NORMAL_ROOM;
	}

	for (int i = 0; i < size; i++)
	{
		position.x += left ? distance : -distance;
		rooms.emplace_back();

		RoomData& roomRef = rooms[rooms.size() - 1];
		glm::vec3& posRef = rooms[rooms.size() - 1].position;

		roomRef.type = roomType;
		posRef = position;

		int curRoomIndex;
		int sideIndex = (int)rooms.size() - 1;
		if (left)
		{
			if (i == 0)
			{
				curRoomIndex = index;
				rooms[curRoomIndex].left = sideIndex;
				rooms[sideIndex].right = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				rooms[curRoomIndex].left = sideIndex;
				rooms[sideIndex].right = curRoomIndex;
			}
			connections.emplace_back(sideIndex, curRoomIndex);
		}
		else
		{
			if (i == 0)
			{
				curRoomIndex = index;
				rooms[curRoomIndex].right = sideIndex;
				rooms[sideIndex].left = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				rooms[curRoomIndex].right = sideIndex;
				rooms[sideIndex].left = curRoomIndex;
			}
			connections.emplace_back(curRoomIndex, sideIndex);
		}


	}
}

bool RoomLayout::setExit()
{
	int exitIndex = rand() % (rooms.size() - 1) + 1;
	int numTests = 0;
	while (rooms[exitIndex].type == RoomData::Type::BOSS_ROOM || rooms[exitIndex].type == RoomData::Type::START_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (rooms[exitIndex].left != -1 && rooms[rooms[exitIndex].left].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (rooms[exitIndex].right != -1 && rooms[rooms[exitIndex].right].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	rooms[exitIndex].type = RoomData::Type::EXIT_ROOM;

	return true;
}

const RoomLayout::RoomData& RoomLayout::getRoom(int index)
{
	return rooms[index];
}

int RoomLayout::getNumRooms() const
{
	return (int)rooms.size();
}

int RoomLayout::getNumMainRooms() const
{
	return numMainRooms;
}

const std::vector<glm::ivec2>& RoomLayout::getConnections()
{
	return connections;
}