#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class RoomLayout
{
public:
	struct RoomData
	{
		enum Type{START_ROOM, NORMAL_ROOM, HARD_ROOM, BOSS_ROOM, EXIT_ROOM};
		
		Type type;
		glm::vec3 position;
	
		// Index of connected rooms
		int up, down, left, right;
		RoomData()
			:up(-1), down(-1), left(-1), right(-1),
			type(Type::NORMAL_ROOM), position(0.f)
		{
		};
	};

	static const int MIN_MAIN_ROOMS = 2;
	static const int MAX_MAIN_ROOMS = 4;

	static const int MIN_NUM_BRANCHES = 1;
	static const int MAX_NUM_BRANCHES = 3; // Cannot exceed (MAX_MAIN_ROOMS * 2)

	static const int MIN_BRANCH_SIZE = 1;
	static const int MAX_BRANCH_SIZE = 2;

private:
	float distance;

	int numMainRooms;
	int numBranches;
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