#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;
class ResourceManager;

class RoomHandler
{
public:
	static const float OUT_OF_BOUNDS;
	static const float ROOM_WIDTH;

private:
	RoomGenerator roomGenerator;
	RoomLayout roomLayout;
	Scene* scene;
	ResourceManager* resourceMan;

	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t doorMeshId;

	bool hasDoor[4];

	float roomGridSize;
	float roomWidth;
	int numRooms;

	int createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos);
	int createDoorEntity(float yRotation);
	void createDoors(int roomIndex, float tileScale);
	void createPathway(int direction);

#ifdef _DEBUG
	void reload();

	// Used for reloading
	std::vector<int> tileIds;
	std::vector<int> doors;
#endif

public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);

	void generate();
	void update();
};