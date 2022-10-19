#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Scene;

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

		bool branch, branchEnd, shortcut;

		RoomData()
			:up(-1), down(-1), left(-1), right(-1),
			branch(false), branchEnd(false), shortcut(false),
			type(Type::NORMAL_ROOM), position(0.f)
		{
		};
	};

private:
	Scene* scene;

	float distance;

	int boss;
	bool foundBoss;
	int roomID;
	int numMainRooms;

	std::vector<RoomData> rooms;
	std::vector<glm::ivec2> connections;


	// Private Functions
	void setUpRooms(int numRooms);

	bool setRandomBranch(int numRooms);
	void setBranch(int index, bool left, int size);

	bool setBoss(int numRooms);
	bool setExit();
	bool setShortcut(int numBranches, int numRooms);
	
	int getEndWithRightAvaliable();
	int getEndWithLeftAvaliable();
	

#if PLAY
	bool canGoForward();
	bool canGoBack();
	bool canGoLeft();
	bool canGoRight();

	void fightBoss(int& boss, int& bossHealth, int& roomID, bool& foundBoss);
	void printDoorOptions(int& roomID);
	void traverseRoomsConsole();
#endif

public:
	RoomLayout();
	virtual ~RoomLayout();

	void init(Scene* scene, float distance);

	void generate();
	void clear();

	const RoomData& getRoom(int index)
	{
		return rooms[index];
	}
	int getNumRooms() const
	{
		return (int)rooms.size();
	}
	int getNumMainRooms() const
	{
		return numMainRooms;
	}

	std::vector<glm::ivec2>& getConnections() 
	{
		return connections;
	}

	bool traverseRooms(int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta);
};