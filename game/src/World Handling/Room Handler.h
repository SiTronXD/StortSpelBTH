#pragma once

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
	static const uint32_t DECO_ENTITY_CHANCE;
private:

	// Helper structs
	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 positions[4]{};
	};

	// CHANGE roomTiles.emplace_back() to roomTiles[i] = ()
	// CHANGE roomTiles.emplace_back() to roomTiles[i] = ()
	// CHANGE roomTiles.emplace_back() to roomTiles[i] = ()

	struct Room
	{
		Room()
			:doors{-1,-1,-1,-1}, doorTriggers{-1,-1,-1,-1},
			connectingIndex{-1,-1,-1,-1}, finished(false), 
			type(RoomData::INVALID), position(0.f)
			, extents{}, rock(-1), rockFence(-1)
		{		
		}

		glm::vec3 position;
		float extents[4];
		RoomData::Type type;

		std::vector<glm::vec3> mainTiles; // "Playable" tiles (used for spawning enemies)
		std::vector<Entity> objects;	  // Objects inside room (borders, rocks etc)

		Entity doors[4];
		Entity doorTriggers[4];
		int connectingIndex[4];

		bool finished;
		Entity rock;
		Entity rockFence;
	};

	// Scene
	Scene* scene;
	ResourceManager* resourceMan;

	// Layout generation
	std::vector<bool> verticalConnection;
	std::vector<std::pair<glm::vec3, glm::vec3>> exitPairs;


	// Room generation
	int tileFloorMeshId;
	void placeBranch(int index, int left, int right);
	void moveRoom(int roomIndex, const glm::vec3& offset);
	std::vector<RoomExitPoint> roomExitPoints;
	bool hasDoor[4]; // remove?
	
	// Create Entities
	Entity createFloorDecoEntity(const glm::vec2& pos, bool scalePos);
	Entity createBorderEntity(const glm::vec2& position, bool scalePos);
	Entity createObjectEntity(const Tile2& tile);
	Entity createDoorEntity(float yRotation);

	// Tile creation
	void createDoors(int roomIndex, const glm::ivec2* doorTilePos);
	void setConnections(int numMainRooms, const std::vector<glm::ivec2>& connections);
	void generatePathways();
	void surroundPaths(const std::vector<glm::vec3>& pathPos, glm::vec3 p0, glm::vec3 p1, float distFactor, bool vertical, bool colliders);

	// IDs
	std::vector<Room> rooms;
	std::vector<Entity> pathEntities;
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
	std::vector<std::pair<uint32_t, uint32_t>> oneXTwoMeshIds;
	std::vector<std::pair<uint32_t, uint32_t>> twoXTwoMeshIds;
	std::vector<uint32_t> borderMeshIds;
	uint32_t innerBorderMesh;
	uint32_t rockMeshId;
	uint32_t rockFenceMeshId;
	uint32_t doorMeshID;

	// Other
	void createColliders();

#ifdef _CONSOLE
	void reset();
	bool showAllRooms = false;
#endif
	
public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan);
	void generate2();

#ifdef  _CONSOLE
	void imgui(PhysicsEngine* physicsEngine = nullptr);
#endif //  _CONSOLE

	void roomCompleted();
	bool onPlayerTrigger(Entity otherEntity);

	const std::vector<glm::vec3>& getFreeTiles();
	const Room& getExitRoom() const;
	int getNumRooms() const;

	Entity getFloor() const;
};