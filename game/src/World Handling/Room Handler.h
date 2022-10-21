#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;
class ResourceManager;
typedef int Entity;

// temp before bullet
struct Box2D
{
	glm::vec2 extents = glm::vec2(0.f);
	bool colliding(const glm::vec3& boxPos, const glm::vec3& otherPos)
	{
		glm::vec3 delta = otherPos - boxPos;
		return std::abs(delta.x) <= extents.x && std::abs(delta.z) <= extents.y;
	}
};

class RoomHandler
{
public:
	static const float TILE_WIDTH;
	static const uint32_t TILES_BETWEEN_ROOMS;

private:
	Scene* scene;
	ResourceManager* resourceMan;
	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t openDoorMeshID;
	uint32_t closedDoorMeshID;

	RoomGenerator roomGenerator;
	RoomLayout roomLayout;

	int roomGridSize;
	
	bool hasDoor[4];

	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;

	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 worldPositions[4]{};
	};
	std::vector<RoomExitPoint> roomExitPoints;

	Entity createTileEntity(int tileIndex, const glm::vec3& roomPos);
	Entity createDoorEntity(float yRotation, const glm::vec3& offset);
	Entity createPathEntity();
	
	void createDoors(int roomIndex);
	void createConnectionPoint(int roomIndex, const glm::vec3& roomPos);
	void createPathways();

	void reset();

	struct Room
	{
		Room()
			:doorIds{-1,-1,-1,-1}
		{		
		}

		std::vector<Entity> tileIds;

		Entity doorIds[4];
		int connectingIndex[4];
	};

	std::vector<Room> rooms;
	std::vector<Entity> pathIds;




	// temp before slay 
	bool roomFinished = false;


	int activeIndex = 0;
	int nextIndex = -1;
	int curDoor = -1;
	bool insideDoor = false;

	void checkRoom(int index, const glm::vec3& playerPos);
	void setActiveRooms();
	void flipDoors(bool open);

public:
	static glm::vec3 snapToGrid(const glm::vec3& pos);

	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);

	void generate();
	void update(const glm::vec3& playerPos);
};