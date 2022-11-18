#include "Room Layout.h"
#include "vengine/dev/Log.hpp"

#include <ctime>

RoomLayout::RoomLayout()
	:distance(0.f), numMainRooms(0), NUM_BRANCHES(0), largestBranchSize(0)
{
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
	this->numMainRooms = 0;
	this->NUM_BRANCHES = 0;
	this->largestBranchSize = 0;

	this->rooms.clear();
	this->connections.clear();
}

void RoomLayout::generate()
{
	this->numMainRooms = rand() % ((MAX_MAIN_ROOMS - MIN_MAIN_ROOMS) + 1) + MIN_MAIN_ROOMS;
	this->NUM_BRANCHES = rand() % ((MAX_NUM_BRANCHES - MIN_NUM_BRANCHES) + 1) + MIN_NUM_BRANCHES;

	this->connections.reserve(size_t(this->numMainRooms + this->NUM_BRANCHES) - 1ull);

	this->setUpRooms(this->numMainRooms);
	for (int i = 0; i < this->NUM_BRANCHES; i++)
	{
		if (!this->setRandomBranch(this->numMainRooms)) 
		{
			Log::warning("Failed creating branch");
		}
	}

	if (!this->setExit())
	{
		Log::warning("Failed creating exit");
	}
}

void RoomLayout::setUpRooms(int numRooms)
{
	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numRooms; i++)
	{
		this->rooms.emplace_back();

		RoomData& curRoom = this->rooms[i];
		glm::vec3& curPosition = curRoom.position;

		curPosition.z = i * this->distance;

		//First room is always the start room
		if (i == 0)
		{
			curRoom.type = RoomData::Type::START_ROOM;
		}
		else
		{
			if (i > 0)
			{
				this->rooms[size_t(i - 1)].up = i;
				this->rooms[i].down = i - 1;

				this->connections.emplace_back(i, i - 1);
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
	int branchSize = rand() % ((MAX_BRANCH_SIZE - MIN_BRANCH_SIZE) + 1) + MIN_BRANCH_SIZE;

	if (branchSize > this->largestBranchSize)
	{
		this->largestBranchSize = branchSize;
	}

	int spot = 0;
	if (numRooms > 1)
	{
		spot = rand() % (numRooms - 1);
	}

	bool foundSpot = false;
	int numTest = 0;

	if (this->rooms[spot].left != -1 && this->rooms[spot].right != -1)
	{
		//Keep looking for a spot to place branch
		while (!foundSpot)
		{
			if (this->rooms[spot].left == -1 || this->rooms[spot].right == -1)
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
	if (this->rooms[spot].left == -1 && this->rooms[spot].right == -1)
	{
		if (rand() % 2 == 0)
		{
			this->setBranch(spot, true, branchSize);
		}
		else
		{
			this->setBranch(spot, false, branchSize);
		}
	}
	else if (this->rooms[spot].left == -1)
	{
		this->setBranch(spot, true, branchSize);
	}
	else
	{
		this->setBranch(spot, false, branchSize);
	}

	return true;
}

void RoomLayout::setBranch(int index, bool left, int size)
{
	RoomData::Type roomType = RoomData::Type::NORMAL_ROOM;

	glm::vec3 position = this->rooms[index].position;

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
		position.x += left ? this->distance : -this->distance;
		rooms.emplace_back();

		RoomData& roomRef = this->rooms[this->rooms.size() - 1];
		glm::vec3& posRef = this->rooms[this->rooms.size() - 1].position;

		roomRef.type = roomType;
		posRef = position;

		int curRoomIndex;
		int sideIndex = (int)this->rooms.size() - 1;
		if (left)
		{
			if (i == 0)
			{
				curRoomIndex = index;
				this->rooms[curRoomIndex].left = sideIndex;
				this->rooms[sideIndex].right = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)this->rooms.size() - 2;
				this->rooms[curRoomIndex].left = sideIndex;
				this->rooms[sideIndex].right = curRoomIndex;
			}
			this->connections.emplace_back(sideIndex, curRoomIndex);
		}
		else
		{
			if (i == 0)
			{
				curRoomIndex = index;
				this->rooms[curRoomIndex].right = sideIndex;
				this->rooms[sideIndex].left = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)this->rooms.size() - 2;
				this->rooms[curRoomIndex].right = sideIndex;
				this->rooms[sideIndex].left = curRoomIndex;
			}
			this->connections.emplace_back(curRoomIndex, sideIndex);
		}


	}
}

bool RoomLayout::setExit()
{
	int exitIndex = rand() % (this->rooms.size() - 1) + 1;
	int numTests = 0;
	while (this->rooms[exitIndex].type == RoomData::Type::BOSS_ROOM || this->rooms[exitIndex].type == RoomData::Type::START_ROOM)
	{
		if (++exitIndex >= this->rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > this->rooms.size()) {
			return false;
		}
	}
	while (this->rooms[exitIndex].left != -1 && this->rooms[this->rooms[exitIndex].left].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= this->rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > this->rooms.size()) {
			return false;
		}
	}
	while (this->rooms[exitIndex].right != -1 && this->rooms[this->rooms[exitIndex].right].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= this->rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > this->rooms.size()) {
			return false;
		}
	}
	this->rooms[exitIndex].type = RoomData::Type::EXIT_ROOM;

	return true;
}

const RoomData& RoomLayout::getRoom(int index)
{
	return this->rooms[index];
}

int RoomLayout::getNumRooms() const
{
	return (int)this->rooms.size();
}

int RoomLayout::getNumMainRooms() const
{
	return this->numMainRooms;
}

int RoomLayout::getLargestBranch() const
{
	return this->largestBranchSize;
}

const std::vector<glm::ivec2>& RoomLayout::getConnections()
{
	return this->connections;
}