#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
	
struct RoomData
{
	enum Type : int
	{
		INVALID = -1,
		START_ROOM, 
		NORMAL_ROOM,
		HARD_ROOM,
		BOSS_ROOM, 
		EXIT_ROOM
	};
	
	Type type;
	glm::vec3 position;

	// Index of connected rooms
	int up, down, left, right;
	RoomData()
		:up(-1), down(-1), left(-1), right(-1),
		type(Type::INVALID), position(0.f)
	{
	};
};

class RoomLayout
{
public:

	static const int MIN_MAIN_ROOMS = 1;
	static const int MAX_MAIN_ROOMS = 1;

	static const int MIN_NUM_BRANCHES = 1;
	static const int MAX_NUM_BRANCHES = 1; // Will cap at numMainRooms * 2

	static const int MIN_BRANCH_SIZE = 1;
	static const int MAX_BRANCH_SIZE = 1;

	// Total minimum rooms: 
	// MIN_MAIN_ROOMS + MIN_NUM_BRANCHES * MIN_BRANCH_SIZE

	// Total maximum rooms:
	// MAX_MAIN_ROOMS + MAX_NUM_BRANCHES * MAX_BRANCH_SIZE

private:
	float distance;

	int numMainRooms;
	int NUM_BRANCHES;
	int largestBranchSize;

	std::vector<RoomData> rooms;
	std::vector<glm::ivec2> connections;

	void setUpRooms(int numRooms);
	bool setRandomBranch(int numRooms);
	void setBranch(int index, bool left, int size);

	bool setExit();

public:
	RoomLayout();
	virtual ~RoomLayout();

	void setRoomDistance(float distance);

	void generate();
	void clear();

	const RoomData& getRoom(int index);
	int getNumRooms() const;
	int getNumMainRooms() const;
	int getLargestBranch() const;
	const std::vector<glm::ivec2>& getConnections();
};