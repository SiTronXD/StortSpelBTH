#pragma once

#include "Room Layout.h"
#include "Room Generator.h"
#include "vengine/components/Collider.h"
#include <array>


class VRandom;
class DebugRenderer;
class PhysicsEngine;
class Scene;
class ResourceManager;
typedef int Entity;

struct EdgeTile
{
    char dummy{};
};

struct TileInfo 
{
public: 
    friend class RoomHandler;
    static const int NONE  = -1;
    static const int LEFT  =  0;
    static const int RIGHT =  1;
    static const int DOWN  =  2;
    static const int UP    =  3;

private:
    const std::array<int,4> neighbours;
    glm::vec3 pos;

    bool amIMyNeighboursNeighbour(int myID, const std::vector<TileInfo>& allTiles) const;

public:     
    TileInfo(const glm::vec3 pos, std::array<int,4>&& neighbours)
        : pos(pos), neighbours(neighbours)
    {}

    const int& idLeftOf() const {return neighbours[LEFT]  ;};
    const int& idRightOf()const {return neighbours[RIGHT] ;};
    const int& idDownOf() const {return neighbours[DOWN]  ;};
    const int& idUpOf()   const {return neighbours[UP]    ;};

    inline const glm::vec3& getPos() const  {return pos;};
    static bool checkValidTileInfoVector(const std::vector<TileInfo>& allTiles, int roomIndex);

};

class RoomHandler
{
public:
	static const float TILE_WIDTH;
	static const float BORDER_COLLIDER_HEIGHT;
	static const uint32_t TILES_BETWEEN_ROOMS;
	static const uint32_t DECO_ENTITY_CHANCE;
	
	static const uint32_t NUM_BORDER;
	static const uint32_t NUM_ONE_X_ONE;
	static const uint32_t NUM_ONE_X_TWO;
	static const uint32_t NUM_TWO_X_TWO;

	static const glm::vec3 DOOR_LAMP_OFFSET;
	static const glm::vec3 DOOR_LAMP_COLOUR;
	static const float DOOR_LAMP_INTENSITY;
	static const float FLICKER_INTERVAL;
	static const int FLICKER_INTENSITY;
private:

	// Helper structs
	struct RoomExitPoint
	{
		RoomExitPoint() = default;
		glm::vec3 positions[4]{};
	};

	struct Room
	{
		Room()
			:doors{-1,-1,-1,-1}, connectingIndex{-1,-1,-1,-1}, 
			finished(false), type(RoomData::INVALID), position(0.f)
			, extents{}, rock(-1), rockFence(-1), colliderPos(0.f)
		{		
		}

		glm::vec3 position;
		float extents[4];
		RoomData::Type type;
		
        std::vector<TileInfo>  tileInfos;
		std::vector<glm::vec3> mainTiles; // "Playable" tiles (used for spawning enemies)
		std::vector<Entity> objects;	  // Objects inside room (borders, rocks etc)

		glm::vec3 colliderPos;
		Collider box;
		Entity doors[4];
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
	void placeBranch(int index, int left, int right);
	void moveRoom(int roomIndex, const glm::vec3& offset);
	std::vector<RoomExitPoint> roomExitPoints;
	
	// Create Entities
	Entity createFloorDecoEntity(const glm::vec2& pos, bool scalePos);
	Entity createBorderEntity(const glm::vec2& position, bool scalePos);
	void createObjectEntities(const Tile& tile, Room& room);
	Entity createDoorEntity(float yRotation);

    // Create TileInfos 
    void createTileInfos(uint32_t roomIndex);

	// Doors and paths
	void createDoors(int roomIndex, const glm::ivec2* doorTilePos);
	void setConnections(int numMainRooms, const std::vector<glm::ivec2>& connections);
	void generatePathways();
	void surroundPaths(size_t startIdx, const std::vector<glm::vec3>& pathPos, glm::vec3 p0, glm::vec3 p1, float distFactor, bool vertical, bool colliders);

	// IDs
	std::vector<Room> rooms;
	std::vector<Entity> pathEntities;
	Entity floor;
	Entity doorLamps[4];

	// Room Updating
	int respawnDoorIdx = -1;
	int prevRoomIndex = -1;
	int activeIndex = 0;
	void showPaths(bool show);
	void closeDoors(int index);
	void activateRoom(int index);
	void deactivateRoom(int index);
	void placeDoorLamps();

	// Resource IDs
	std::vector<int> oneXOneMeshIds;
	std::vector<std::pair<int, int>> oneXTwoMeshIds;
	std::vector<std::pair<int, int>> twoXTwoMeshIds;
	std::vector<int> borderMeshIds;
	int innerBorderMesh;
	int rockMeshId;
	int rockFenceMeshId;
	int doorMeshID;
	int tileFloorMeshId;
	int lampMeshId;
	int lampDiffuseId;
	int lampGlowId;

	// Other
	void createFloor();
	void reset();
	VRandom* random; // Created and deleted in generate()
	bool useMeshes; // Required by server
	float flickerTimer = 0.f;


public:
	RoomHandler();
	~RoomHandler();

	void init(Scene* scene, ResourceManager* resourceMan, bool useMeshes);
	void generate(uint32_t seed);

#ifdef _CONSOLE
	void imgui(DebugRenderer* dr);
#endif //  _CONSOLE

	void roomCompleted();
	bool playerNewRoom(Entity player, PhysicsEngine* physicsEngine);
	void startOver();
	glm::vec3 getRespawnPos() const;
	glm::vec3 getRespawnRot() const;

	const std::vector<glm::vec3>& getFreeTiles();
	const std::vector<TileInfo>& getFreeTileInfos();
	const Room& getExitRoom() const;
	int getNumRooms() const;

    const glm::vec3& getRoomPos() const;

	Entity getFloor() const;

	std::vector<std::vector<glm::vec3>> getPathFindingPoints();
};