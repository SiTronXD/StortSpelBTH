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

	// Helper structs
	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 worldPositions[4]{};
	};

	struct Room
	{
		Room()
			:doorIds{-1,-1,-1,-1},
			connectingIndex{-1,-1,-1,-1}
		{		
		}

		std::vector<Entity> tileIds;

		Entity doorIds[4];
		int connectingIndex[4];
	};

	// Scene
	Scene* scene;
	ResourceManager* resourceMan;

	// Layout generation
	RoomLayout roomLayout;
	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;

	// Room generation
	RoomGenerator roomGenerator;
	std::vector<RoomExitPoint> roomExitPoints;
	bool hasDoor[4];
	
	// Create Entities
	Entity createTileEntity(int tileIndex, const glm::vec3& roomPos);
	Entity createDoorEntity(float yRotation, const glm::vec3& offset);
	Entity createPathEntity();
	
	// Create tiles
	void createDoors(int roomIndex);
	void createConnectionPoint(int roomIndex, const glm::vec3& roomPos);
	void createPathways();

	// IDs
	std::vector<Room> rooms;
	std::vector<Entity> pathIds;
	
	// Room Updating
	int activeIndex = 0;
	int nextIndex = -1;
	int curDoor = -1;
	bool insideDoor = false;
	bool roomFinished = false; // temp before slayyyy queen yaasss

	void checkRoom(int index, const glm::vec3& playerPos);
	void setActiveRooms();
	void flipDoors(bool open);

	// Mesh IDs
	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t openDoorMeshID;
	uint32_t closedDoorMeshID;

	// Other helpful stuff :]
	void reset();
	void scaleRoom();

public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);
	void generate();
	void update(const glm::vec3& playerPos);

	// Statics
	static glm::vec3 snapToGrid(const glm::vec3& pos);
};