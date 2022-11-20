#include "Room Layout.h"
#include "vengine/dev/Log.hpp"
#include "vengine/dev/Random.hpp"

RoomLayout::RoomLayout(VRandom& random)
	:numMainRooms(0), numBranches(0), largestBranchSize(0), random(random)
{
}

RoomLayout::~RoomLayout()
{
}

void RoomLayout::clear()
{
	this->numMainRooms = 0;
	this->numBranches = 0;
	this->largestBranchSize = 0;

	this->rooms.clear();
	this->connections.clear();
}

void RoomLayout::generate()
{
	this->numMainRooms = this->random.rand() % ((MAX_MAIN_ROOMS - MIN_MAIN_ROOMS) + 1) + MIN_MAIN_ROOMS;
	this->numBranches = this->random.rand() % ((MAX_NUM_BRANCHES - MIN_NUM_BRANCHES) + 1) + MIN_NUM_BRANCHES;

	this->connections.reserve(size_t(this->numMainRooms + this->numBranches) - 1ull);

	this->setUpRooms(this->numMainRooms);
	for (int i = 0; i < this->numBranches; i++)
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
	for (int i = 0; i < numRooms; i++)
	{
		this->rooms.emplace_back();

		RoomData& curRoom = this->rooms[i];

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
			if (this->random.rand() % 5 == 0)
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
	int branchSize = this->random.rand() % ((MAX_BRANCH_SIZE - MIN_BRANCH_SIZE) + 1) + MIN_BRANCH_SIZE;

	if (branchSize > this->largestBranchSize)
	{
		this->largestBranchSize = branchSize;
	}

	int spot = 0;
	if (numRooms > 1)
	{
		spot = this->random.rand() % (numRooms - 1);
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
		if (this->random.rand() % 2 == 0)
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

	if (this->random.rand() % 5 == 0)
	{
		roomType = RoomData::Type::HARD_ROOM;
	}
	else
	{
		roomType = RoomData::Type::NORMAL_ROOM;
	}

	for (int i = 0; i < size; i++)
	{
		this->rooms.emplace_back();

		RoomData& roomRef = this->rooms[this->rooms.size() - 1];

		roomRef.type = roomType;

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
	int exitIndex = this->random.rand() % (this->rooms.size() - 1) + 1;
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