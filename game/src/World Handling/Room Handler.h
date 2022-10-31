#pragma once
#include "Room Generator.h"
#include "Room Layout.h"

class Scene;
class ResourceManager;
typedef int Entity;

class RoomHandler
{
public:
	static const float TILE_WIDTH;
	static const uint32_t TILES_BETWEEN_ROOMS;

private:

	enum TileUsage { Default, Border, Exit };

	// Helper structs
	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 positions[4]{};
	};

	struct Room
	{
		Room()
			:doors{-1,-1,-1,-1}, doorTriggers{-1,-1,-1,-1},
			connectingIndex{-1,-1,-1,-1}, finished(false)
		{		
		}

		std::vector<Entity> tiles;
		std::vector<Entity> borders;
		std::vector<Entity> exitPaths;

		Entity doors[4];
		Entity doorTriggers[4];
		int connectingIndex[4];

		bool finished;
	};

	// Scene
	Scene* scene;
	ResourceManager* resourceMan;

	// Layout generation
	RoomLayout roomLayout;
	std::vector<bool> verticalConnection;
	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;

	// Room generation
	RoomGenerator roomGenerator;
	std::vector<RoomExitPoint> roomExitPoints;
	bool hasDoor[4];
	void setExitPoints(int roomIndex);
	void roomToWorldSpace(int roomIndex);

	// Create Entities
	Entity createTileEntity(int tileIndex, TileUsage usage);
	Entity createDoorEntity(float yRotation);
	Entity createPathEntity();
	Entity createPathBorderEntity(const glm::vec3& position);
	
	// Tile creation
	void createDoors(int roomIndex);
	void setConnections();
	void generatePathways();

	// IDs
	std::vector<Room> rooms;
	std::vector<Entity> pathIds;
	Entity floor;

	// Room Updating
	int activeIndex = 0;
	int nextIndex = -1;
	int curDoor = -1;
	bool checkRoom(int index, Entity entity);

	void showPaths(bool show);
	void openDoors(int index);
	void closeDoors(int index);
	void activateRoom(int index);
	void deactivateRoom(int index);

	// Mesh IDs
	std::unordered_map<Tile::Type, uint32_t> tileMeshIds;
	uint32_t openDoorMeshID;
	uint32_t closedDoorMeshID;

	// Other
	void reset();
	void createColliders();

#ifdef _CONSOLE
	bool showAllRooms = false;
#endif

public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes);
	void generate();

#ifdef  _CONSOLE
	void imgui();
#endif //  _CONSOLE

	void roomCompleted();
	bool onPlayerTrigger(Entity otherEntity);

	const std::vector<Entity>& getFreeTiles();
};