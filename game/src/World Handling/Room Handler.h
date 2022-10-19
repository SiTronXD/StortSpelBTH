#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;
class ResourceManager;

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
	static const float ROOM_WIDTH;

private:
	Scene* scene;
	ResourceManager* resourceMan;
	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t doorMeshId;

	RoomGenerator roomGenerator;
	RoomLayout roomLayout;

	float roomGridSize;
	int numRooms;

	const int numTilesInbetween;
	float tileWidth;
	
	bool hasDoor[4];

	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;

	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 worldPositions[4]{};
	};
	std::vector<RoomExitPoint> roomExitPoints;

	int createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos);
	int createDoorEntity(float yRotation, const glm::vec3& offset);
	int createPathEntity();
	
	void createDoors(int roomIndex, float tileScale);
	void createConnectionPoint(int roomIndex, float tileScale, const glm::vec3& roomPos);
	void createPathways(float tileScale);

	glm::vec3 snapToGrid(const glm::vec3& pos);

	void reset();

	struct Room
	{
		Room()
		{		
			memset(doorIds, -1, sizeof(doorIds));
		}

		std::vector<int> tileIds;

		int doorIds[4];
		int connectingIndex[4];
	};

	std::vector<Room> rooms;
	std::vector<int> pathIds;

	void deactiveRooms();
	int prevIndex;
	int activeIndex;

public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);

	void generate();
	void update(const glm::vec3& playerPos);
};