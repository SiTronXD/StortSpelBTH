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

	float roomGridSize;
	int numRooms;

	float roomWidth;
	const int numTilesInbetween;
	float tileWidth;
	
	bool hasDoor[4];

	struct ConnectionPoints
	{
		ConnectionPoints() = default;

		int index1 = -1;
		glm::vec2 pos1{};

		int index2 = -1;
		glm::vec2 pos2{};
	};

	std::vector<ConnectionPoints> connections;

	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		~RoomExitPoint()
		{
			for (int i = 0; i < 4; i++)
			{
				if (worldPositions[i])
				{
					delete worldPositions[i];
					worldPositions[i] = nullptr;
				}
			}
		}

		glm::vec2* worldPositions[4]{};
	};

	std::vector<RoomExitPoint> exitPoints;

	int createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos);
	int createDoorEntity(float yRotation);
	int createPathEntity();
	
	void createDoors(int roomIndex, float tileScale);
	void createConnectionPoint();
	void createPathways(float tileScale);

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