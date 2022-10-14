#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;
class ResourceManager;

class RoomHandler
{
public:
	static const float ROOM_WIDTH;

private:
	RoomGenerator roomGenerator;
	RoomLayout roomLayout;
	Scene* scene;
	ResourceManager* resourceMan;

	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t doorMeshId;

	float roomGridSize;
	int numRooms;

	const int numTilesInbetween;
	float tileWidth;
	
	bool hasDoor[4];
	int possibleDoors[4];

	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;

	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 worldPositions[4]{};
	};

	std::vector<RoomExitPoint> roomExitPoints;

	int createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos);
	int createDoorEntity(float yRotation);
	int createPathEntity();
	
	void createDoors(int roomIndex, float tileScale);
	void createConnectionPoint(float tileScale, const glm::vec3& roomPos);
	void createPathways();

#ifdef _DEBUG
	void reload();

	// Used for reloading
	std::vector<int> tileIds;
	std::vector<int> doors;
	std::vector<int> connectionsIds;
#endif

public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);

	void generate();
	void update();
};