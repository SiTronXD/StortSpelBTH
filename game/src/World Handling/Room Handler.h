#pragma once
#include "Room Generator.h"
#include "Room Layout.h"
#include "Room Generator2.h"

class Scene;
class ResourceManager;
typedef int Entity;

#ifdef _CONSOLE
class PhysicsEngine;
#endif

class RoomHandler
{
public:
	static const float TILE_WIDTH;
	static const uint32_t TILES_BETWEEN_ROOMS;
	static const uint32_t NUM_BORDER;
	static const uint32_t NUM_ONE_X_ONE;
	static const uint32_t NUM_ONE_X_TWO;
	static const uint32_t NUM_TWO_X_TWO;
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
			connectingIndex{-1,-1,-1,-1}, finished(false), 
			type(RoomData::INVALID), position(0.f)
			, extents{}
		{		
		}

		glm::vec3 position;
		float extents[4];
		RoomData::Type type;

		std::vector<Entity> mainTiles; // "Playable" tiles
		std::vector<Entity> objects; // Occupied tiles + their floors
		std::vector<Entity> borders; // Borders
		std::vector<Entity> exitPaths; // Tiles leading to path

		Entity doors[4];
		Entity doorTriggers[4];
		int connectingIndex[4];

		bool finished;
	};

	// Scene
	Scene* scene;

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

	// Room generation 2
	RoomGen roomGen;
	RoomGen::RoomGenDescription roomGenDesc;
	int tileFlorMeshId;
	void placeBranch(int index, int left, int right);
	void moveRoom(int roomIndex, glm::vec3 offset);
	
	// New Create Entities
	Entity createFloorEntity(const glm::vec2& pos);
	Entity createBorderEntity(const glm::vec2& position, bool scalePos);


	// Create Entities
	Entity createTileEntity(int tileIndex, TileUsage usage);
	Entity createDoorEntity(float yRotation);
	Entity createPathEntity();
	
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
	std::vector<uint32_t> oneXOneMeshIds;
	std::vector<uint32_t> borderMeshIds;
	std::vector<uint32_t> oneXTwoMeshIds;
	std::vector<uint32_t> twoXTwoMeshIds;
	uint32_t doorMeshID;

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
	void generate2();

#ifdef  _CONSOLE
	void imgui(PhysicsEngine* physicsEngine = nullptr);
#endif //  _CONSOLE

	void roomCompleted();
	bool onPlayerTrigger(Entity otherEntity);

	const std::vector<Entity>& getFreeTiles();
	const RoomData::Type& getActiveRoomType() const;
	const Room& getExitRoom() const;
	int getNumRooms() const;

	Entity getFloor() const;
};