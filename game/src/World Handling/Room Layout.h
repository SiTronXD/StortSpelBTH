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
	
		// Index of connected rooms
		int up, down, left, right;

		glm::vec3 position;

		RoomData()
			:up(-1), down(-1), left(-1), right(-1),
			type(Type::NORMAL_ROOM), position(0.f)
		{
		};
	};

private:
	float distance;
	int numMainRooms;

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
	const std::vector<glm::ivec2>& getConnections();
};