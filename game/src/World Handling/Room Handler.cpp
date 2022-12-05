#include "Room Handler.h"
#include "vengine/application/Scene.hpp"
#include "vengine/dev/Random.hpp"
#include "vengine/graphics/DebugRenderer.hpp"
#include "vengine/physics/PhysicsEngine.h"
#include "vengine/application/Input.hpp"
#include "vengine/components/PointLight.hpp"

const float RoomHandler::TILE_WIDTH = 25.f;
const float RoomHandler::BORDER_COLLIDER_HEIGHT = TILE_WIDTH * 6.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 5;
const uint32_t RoomHandler::DECO_ENTITY_CHANCE = 30;
const uint32_t RoomHandler::NUM_BORDER = 1;
const uint32_t RoomHandler::NUM_ONE_X_ONE = 3;
const uint32_t RoomHandler::NUM_ONE_X_TWO = 2;
const uint32_t RoomHandler::NUM_TWO_X_TWO = 1;

const glm::vec3 RoomHandler::DOOR_LAMP_OFFSET = glm::vec3(-12.f, 21.5f, 30.5f);
const glm::vec3 RoomHandler::DOOR_LAMP_COLOUR = glm::vec3(0.94f, 0.28f, 0.05f);
const float RoomHandler::DOOR_LAMP_INTENSITY = 5000.f;
const float RoomHandler::FLICKER_INTERVAL = 0.05f;
const int RoomHandler::FLICKER_INTENSITY = 60;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), activeIndex(0), 
	floor(-1), doorMeshID(0), tileFloorMeshId(0), innerBorderMesh(0),
	rockFenceMeshId(0), rockMeshId(0), random(), useMeshes(true)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, bool useMeshes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->useMeshes = useMeshes;

	if (this->useMeshes)
	{
		this->borderMeshIds.resize(NUM_BORDER);
		for (uint32_t i = 0; i < NUM_BORDER; i++)
		{
			this->borderMeshIds[i] = (int)resourceMan->addMesh("assets/models/Tiles/Border/" + std::to_string(i + 1u) + ".obj");
		}
		this->innerBorderMesh = (int)resourceMan->addMesh("assets/models/Tiles/Border/innerBorder.obj");
	}

	if (this->useMeshes)
	{
		this->oneXOneMeshIds.resize(NUM_ONE_X_ONE);
		for (uint32_t i = 0; i < NUM_ONE_X_ONE; i++)
		{
			this->oneXOneMeshIds[i] = (int)resourceMan->addMesh("assets/models/Tiles/OneXOne/" + std::to_string(i + 1u) + ".obj");
		}
	}

	this->oneXTwoMeshIds.resize(NUM_ONE_X_TWO);
	for (uint32_t i = 0; i < NUM_ONE_X_TWO; i++)
	{
		if (this->useMeshes)
		{
			this->oneXTwoMeshIds[i].first = (int)resourceMan->addMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
		}
		const int collId =(int) resourceMan->addCollisionShapeFromMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
		this->oneXTwoMeshIds[i].second =(int) resourceMan->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;
	}

	this->twoXTwoMeshIds.resize(NUM_TWO_X_TWO);
	for (uint32_t i = 0; i < NUM_TWO_X_TWO; i++)
	{
		if (this->useMeshes)
		{
			this->twoXTwoMeshIds[i].first = resourceMan->addMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
		}
		const int collId = (int)resourceMan->addCollisionShapeFromMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
		this->twoXTwoMeshIds[i].second = (int)resourceMan->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;

	}

	if (this->useMeshes)
	{
		this->doorMeshID = (int)resourceMan->addMesh("assets/models/door.obj");
		this->rockMeshId = (int)resourceMan->addMesh("assets/models/tempRock.obj");
		this->rockFenceMeshId = (int)resourceMan->addMesh("assets/models/rockFence.obj");
		this->tileFloorMeshId = (int)resourceMan->addMesh("assets/models/Tiles/Floor.obj");
		this->lampMeshId = (int)resourceMan->addMesh("assets/models/Tiles/OneXTwo/2.obj");

		this->lampDiffuseId = (int)resourceMan->addTexture("assets/textures/lampTex.png");
		this->lampGlowId = (int)resourceMan->addTexture("assets/textures/Perk_HpTex.png");
	}
}

void RoomHandler::roomCompleted()
{
	Room& curRoom = this->rooms[this->activeIndex];
	curRoom.finished = true;
	if (curRoom.type != RoomData::EXIT_ROOM)
	{
		glm::vec3 pos = scene->getComponent<Transform>(this->rooms[this->activeIndex].rock).position;
		this->scene->setScriptComponent(this->rooms[this->activeIndex].rock, "scripts/moveRock.lua");
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setScriptComponent(curRoom.doors[i], "scripts/opendoor.lua");
			this->scene->removeComponent<Collider>(curRoom.doors[i]);
#ifndef _DEBUG
			this->activateRoom(curRoom.connectingIndex[i]);
#endif
		}
	}

	this->showPaths(true);
}

bool RoomHandler::playerNewRoom(Entity player, PhysicsEngine* physicsEngine)
{
	// Light flicker
	flickerTimer += Time::getDT();
	if (flickerTimer >= FLICKER_INTERVAL)
	{
		flickerTimer = 0.f;
		for (int i = 0; i < 4; i++)
		{
			PointLight& light = this->scene->getComponent<PointLight>(this->doorLamps[i]);
			light.color = DOOR_LAMP_COLOUR * DOOR_LAMP_INTENSITY;
			light.color += rand() % FLICKER_INTENSITY * (rand() % 2 ? 1.f : -1.f);
		}
	}

	Room& curRoom = this->rooms[this->activeIndex];
	if (!curRoom.finished)
	{
		return false;
	}

	std::vector<Entity> entities;
	entities.reserve(10ull);
	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			Room& newRoom = this->rooms[curRoom.connectingIndex[i]];
			entities = physicsEngine->testContact(newRoom.box, newRoom.colliderPos);
			for (const Entity& entity : entities)
			{
				if (entity == player) // Player entered different room
				{
					const int oldIndex = this->activeIndex;
					this->activeIndex = curRoom.connectingIndex[i];
					this->placeDoorLamps();

					if (!newRoom.finished) // Room wasn't finished
					{
						this->closeDoors(this->activeIndex);
						for (int j = 0; j < 4; j++)
						{
							if (newRoom.connectingIndex[j] != -1)
							{
								this->deactivateRoom(newRoom.connectingIndex[j]);
							}
						}
#ifdef _DEBUG
						this->activateRoom(this->activeIndex);
						this->showPaths(false);
#endif // _DEBUG
						return true;
					}

					// Room was finished
					for (int j = 0; j < 4; j++)
					{
						if (curRoom.doors[j] != -1 && curRoom.connectingIndex[j] != this->activeIndex)
						{
							this->deactivateRoom(curRoom.connectingIndex[j]);
						}
#ifndef _DEBUG
						if (newRoom.doors[j] != -1 && newRoom.connectingIndex[j] != oldIndex)
						{
							this->activateRoom(newRoom.connectingIndex[j]);
						}
#else
						this->activateRoom(this->activeIndex);
						this->deactivateRoom(oldIndex);
#endif // !_DEBUG
					}
					return false;
				}
			}
		}
	}
	
	return false;
}

void RoomHandler::generate(uint32_t seed)
{
	const glm::vec3 noDoorBoxOffset[] = 
	{ 
		glm::vec3(TILE_WIDTH, 0.f, 0.f),
		glm::vec3(-TILE_WIDTH, 0.f, 0.f),
		glm::vec3(0.f, 0.f, TILE_WIDTH),
		glm::vec3(0.f, 0.f, -TILE_WIDTH)
	};

	this->random = new(__FILE__, __LINE__) VRandom;
	this->random->srand(seed);

	RoomLayout roomLayout(*this->random);
	RoomGenerator roomGen(*this->random);

	this->reset();

	roomLayout.generate();

	const int numTotRooms = roomLayout.getNumRooms();
	const int numMainRooms = roomLayout.getNumMainRooms();
	this->rooms.resize(numTotRooms);
	this->roomExitPoints.resize(numTotRooms);

	bool hasDoor[4]{};
	for (int i = 0; i < numTotRooms; i++)
	{
		const RoomData& roomData = roomLayout.getRoom(i);
		Room& curRoom = this->rooms[i];

		curRoom.type = roomData.type;

		// Save connecting indices
		curRoom.connectingIndex[LEFT_P]  = roomData.left;
		curRoom.connectingIndex[RIGHT_P] = roomData.right;
		curRoom.connectingIndex[UPPER_P] = roomData.up;
		curRoom.connectingIndex[LOWER_P] = roomData.down;

		// Send door info to roomGen when generating room
		hasDoor[LEFT_P]  = roomData.left != -1;
		hasDoor[RIGHT_P] = roomData.right != -1;
		hasDoor[UPPER_P] = roomData.up != -1;
		hasDoor[LOWER_P] = roomData.down != -1;
		roomGen.generate(hasDoor);

		// Save extents, used when moving rooms and creating floor
		const glm::ivec2* exits = roomGen.getExits();
		curRoom.extents[LEFT_P]  = std::abs((float)exits[LEFT_P].x)  * TILE_WIDTH;
		curRoom.extents[RIGHT_P] = std::abs((float)exits[RIGHT_P].x) * TILE_WIDTH;
		curRoom.extents[UPPER_P] = std::abs((float)exits[UPPER_P].y) * TILE_WIDTH;
		curRoom.extents[LOWER_P] = std::abs((float)exits[LOWER_P].y) * TILE_WIDTH;

		this->createDoors(i, exits);
		const glm::ivec2* roomMinMax = roomGen.getMinMax();

		// Find minMax based on doors, used for the room collider
		glm::vec3 minMax[4]{};
		for (int j = 0; j < 4; j++)
		{
			if (hasDoor[j]) 
			{
				minMax[j] = scene->getComponent<Transform>(curRoom.doors[j]).position; 
			}
			else			
			{ 
				minMax[j] = glm::vec3(roomMinMax[j].x, 0.f, roomMinMax[j].y) * TILE_WIDTH + noDoorBoxOffset[j]; 
			}
		}
		curRoom.colliderPos.x = (minMax[0].x + minMax[1].x) * 0.5f;
		curRoom.colliderPos.y = TILE_WIDTH * 4.f;
		curRoom.colliderPos.z = (minMax[2].z + minMax[3].z) * 0.5f;
		
		curRoom.box.extents.x = (minMax[0].x - minMax[1].x) * 0.5f - TILE_WIDTH;
		curRoom.box.extents.y = TILE_WIDTH * 4.f;
		curRoom.box.extents.z = (minMax[2].z - minMax[3].z) * 0.5f - TILE_WIDTH;

		curRoom.box = Collider::createBox(curRoom.box.extents, curRoom.box.offset, true);

		for (int j = 0; j < 4; j++)
		{
			if (curRoom.connectingIndex[j] != -1)
			{
				this->roomExitPoints[i].positions[j] = glm::vec3(exits[j].x, 0.f, exits[j].y) * TILE_WIDTH;
			}
		}

		Entity entity = -1;
		const std::vector<Tile>& tiles = roomGen.getTiles();
		for (const Tile& tile : tiles)
		{
			switch (tile.type)
			{
			default:
				break;
			case Tile::Border:
			{
				entity = this->createBorderEntity(tile.position, true);
				curRoom.objects.emplace_back(entity);
				break;
			}
			case Tile::InnerBorder:
			{
				entity = this->createBorderEntity(tile.position, true);
				curRoom.objects.emplace_back(entity);
				this->scene->setComponent<Collider>(entity, Collider::createBox(
					glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 3.f, TILE_WIDTH * 0.5f), glm::vec3(0.f, TILE_WIDTH * 3.f, 0.f)));
                    
                this->scene->setComponent<EdgeTile>(entity);
				if (this->useMeshes)
				{
					this->scene->getComponent<MeshComponent>(entity).meshID = this->innerBorderMesh;
				}
				break;
			}
			case Tile::OneXOne:
				if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
				{
					curRoom.objects.emplace_back(this->createFloorDecoEntity(tile.position, true));
				}
				break;

			case Tile::TwoXOne:
				this->createObjectEntities(tile, curRoom);
				break;

			case Tile::OneXTwo:
				this->createObjectEntities(tile, curRoom);
				break;

			case Tile::TwoXTwo:
				this->createObjectEntities(tile, curRoom);
				break;

			case Tile::Exit:
				if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
				{
					curRoom.objects.emplace_back(this->createFloorDecoEntity(tile.position, true));
				}
				break;

			case Tile::AI:
				if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
				{
					curRoom.objects.emplace_back(this->createFloorDecoEntity(tile.position, true));
				}
				
				curRoom.mainTiles.emplace_back(tile.position.x, 0.f, tile.position.y);
				curRoom.mainTiles.back() *= TILE_WIDTH;
				break;
            
			}
		}

		// Spawn big rock if room isn't start or exit
		if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
		{
			Entity entity = this->scene->createEntity();
			curRoom.rockFence = entity;
			this->scene->getComponent<Transform>(entity).position += glm::vec3(TILE_WIDTH * 0.5f, 0.f, TILE_WIDTH * 0.5f);
			this->scene->setComponent<Collider>(entity, Collider::createBox(
					glm::vec3(TILE_WIDTH), glm::vec3(0.f, TILE_WIDTH, 0.f)));
			if (this->useMeshes)
			{
				this->scene->setComponent<MeshComponent>(entity, (int)this->rockFenceMeshId);
			}

			entity = this->scene->createEntity();
			Transform& rockTra = this->scene->getComponent<Transform>(entity);
			rockTra.position = glm::vec3(8.15f, -36.5f, -7.2f);
			rockTra.position += glm::vec3(TILE_WIDTH * 0.5f, 0.f, TILE_WIDTH * 0.5f);
			rockTra.rotation = glm::vec3(11.0f, 0.5f, 12.7f);
			curRoom.rock = entity;
			if (this->useMeshes)
			{
				this->scene->setComponent<MeshComponent>(entity, (int)rockMeshId);
			}

		}
        this->createTileInfos(i);
		roomGen.clear();
	}

	// Move rooms to world space
	for (int i = 0; i < numMainRooms; i++)
	{
		if (this->rooms[i].connectingIndex[LOWER_P] != -1)
		{
			Room& other = this->rooms[rooms[i].connectingIndex[LOWER_P]];
			const float offset = other.position.z + other.extents[UPPER_P] + this->rooms[i].extents[LOWER_P]
				+ TILES_BETWEEN_ROOMS * TILE_WIDTH;

			this->moveRoom(i, glm::vec3(0.f, 0.f, offset));
		}
		this->placeBranch(i, this->rooms[i].connectingIndex[LEFT_P], this->rooms[i].connectingIndex[RIGHT_P]);
	}

	// Create pathways and floor
	this->setConnections(roomLayout.getNumMainRooms(), roomLayout.getConnections());
	this->generatePathways();
	this->createFloor();
	delete this->random;

	// Offset everything so origo is in the middle of spawn tile
	for (Entity entity : this->pathEntities)
	{
		this->scene->getComponent<Transform>(entity).position += glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f);
	}
	for (int i = 0; i < numTotRooms; i++)
	{
		this->moveRoom(i, glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f));

		this->rooms[i].mainTiles.shrink_to_fit();
		this->rooms[i].objects.shrink_to_fit();
	}

	roomLayout.clear();
	this->exitPairs.clear();
	this->roomExitPoints.clear();
	this->verticalConnection.clear();

	this->exitPairs.shrink_to_fit();
	this->roomExitPoints.shrink_to_fit();
	this->verticalConnection.shrink_to_fit();

	for (int i = 0; i < 4; i++)
	{
		this->doorLamps[i] = this->scene->createEntity();

		this->scene->setComponent<PointLight>(this->doorLamps[i]);
		PointLight& light = this->scene->getComponent<PointLight>(this->doorLamps[i]);
		light.color = DOOR_LAMP_COLOUR * DOOR_LAMP_INTENSITY;
		light.positionOffset = DOOR_LAMP_OFFSET;
	}
	this->activeIndex = 0;
	this->placeDoorLamps();
	Room& startRoom = this->rooms[this->activeIndex];
	startRoom.finished = true;

	for (int i = 1; i < numTotRooms; i++)
	{
		this->deactivateRoom(i);	
	}

	for (size_t i = 0; i < this->rooms.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (this->rooms[i].doors[j] != -1)
			{
				this->scene->setScriptComponent(this->rooms[i].doors[j], "scripts/opendoor.lua");
#ifndef _DEBUG
				if (i == 0)
				{
					this->activateRoom(startRoom.connectingIndex[j]);
				}
#endif // !_DEBUG
			}
		}
	}
}

void RoomHandler::moveRoom(int roomIndex, const glm::vec3& offset)
{
	Room& curRoom = this->rooms[roomIndex];

	curRoom.position += offset;
	curRoom.colliderPos += offset;
	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->getComponent<Transform>(curRoom.rock).position += offset;
		this->scene->getComponent<Transform>(curRoom.rockFence).position += offset;
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->roomExitPoints[roomIndex].positions[i] += offset;
			this->scene->getComponent<Transform>(curRoom.doors[i]).position += offset;
		}
	}

    for (TileInfo& tile : curRoom.tileInfos)
	{
		tile.pos += offset;
	}

	for (glm::vec3& pos : curRoom.mainTiles)
	{
		pos += offset;
	}
	for (Entity entity : curRoom.objects)
	{
		this->scene->getComponent<Transform>(entity).position += offset;
	}
}

void RoomHandler::placeBranch(int index, int left, int right)
{
	Room& curRoom = this->rooms[index];
	glm::vec3 offset = curRoom.position;

	if (left != -1)
	{
		Room& leftRoom = this->rooms[left];
		offset.x += curRoom.extents[LEFT_P] + leftRoom.extents[RIGHT_P] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		this->moveRoom(left, offset);

		if (leftRoom.connectingIndex[LEFT_P] != -1)
		{
			this->placeBranch(left, leftRoom.connectingIndex[LEFT_P], -1);
		}
	}

	offset = curRoom.position;
	if (right != -1)
	{
		Room& rightRoom = this->rooms[right];
		offset.x -= curRoom.extents[RIGHT_P] + rightRoom.extents[LEFT_P] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		this->moveRoom(right, offset);

		if (rightRoom.connectingIndex[RIGHT_P] != -1)
		{
			this->placeBranch(right, -1, rightRoom.connectingIndex[RIGHT_P]);
		}
	}
}

Entity RoomHandler::createFloorDecoEntity(const glm::vec2& pos, bool scalePos)
{
	Entity entity = scene->createEntity();
	Transform& transform = scene->getComponent<Transform>(entity);
	transform.position.x = pos.x * (scalePos ? TILE_WIDTH : 1.f) + float((int)this->random->rand() % 12 - 6);
	transform.position.z = pos.y * (scalePos ? TILE_WIDTH : 1.f) + float((int)this->random->rand() % 12 - 6);
	transform.rotation.y = float(this->random->rand() % 360);
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->oneXOneMeshIds[this->random->rand() % NUM_ONE_X_ONE]);
	}
	else
	{
		this->random->rand();
	}
	return entity;
}

const std::vector<glm::vec3>& RoomHandler::getFreeTiles()
{
	return this->rooms[this->activeIndex].mainTiles;
}

const std::vector<TileInfo>& RoomHandler::getFreeTileInfos()
{
	return this->rooms[this->activeIndex].tileInfos;
}

const RoomHandler::Room& RoomHandler::getExitRoom() const
{
	for (const RoomHandler::Room& room : this->rooms)
	{
		if (room.type == RoomData::Type::EXIT_ROOM)
		{
			return room;
		}
	}

	return this->rooms[0];
}

int RoomHandler::getNumRooms() const
{
	return (int)this->rooms.size();
}

bool RoomHandler::inExitRoom() const
{
	return this->rooms[this->activeIndex].type == RoomData::Type::EXIT_ROOM;
}

bool TileInfo::checkValidTileInfoVector(const std::vector<TileInfo>& tileInfos, int roomIndex)
{
    bool tileInfosValid = true; 
    for(size_t i = 0; i < tileInfos.size(); i++ )
    {
        if(!tileInfos[i].amIMyNeighboursNeighbour(i, tileInfos))
        {
            Log::warning("Tile["+std::to_string(i)+"] neigbhours is not neigbhour with Tile["+std::to_string(i)+"]!");
            tileInfosValid = false;
        }
    }

    if(!tileInfosValid)
    {Log::error("RoomHandler has invalid TileInfos for room["+std::to_string(roomIndex)+"], se warning/s above!");}
    else{Log::write("RoomHandlers TileInfos are correct for room["+std::to_string(roomIndex)+"]!");}

    return tileInfosValid;
}

bool TileInfo::amIMyNeighboursNeighbour(int myID, const std::vector<TileInfo>& allTiles) const
{    
    bool goodNeighbour = true;
   
    if(  this->idUpOf() != TileInfo::NONE && allTiles[this->idUpOf()].idDownOf() != myID)
    {
        goodNeighbour = false;
    }
    if( this->idDownOf() != TileInfo::NONE && allTiles[this->idDownOf()].idUpOf() != myID)
    {
        goodNeighbour = false;
    }
    if( this->idLeftOf() != TileInfo::NONE && allTiles[this->idLeftOf()].idRightOf() != myID)
    {
        goodNeighbour = false;
    }
    if( this->idRightOf() != TileInfo::NONE && allTiles[this->idRightOf()].idLeftOf() != myID )
    {
        goodNeighbour = false;
    }

	//NOTE: Without this check we will be able to spawn on tiles without neighbours, if this is a problem we should do something about it later... 
    // if( this->idUpOf() == TileInfo::NONE  && this->idDownOf() == TileInfo::NONE  && this->idLeftOf() == TileInfo::NONE && this->idRightOf() == TileInfo::NONE) 
    // {
    //     assert(false);
    //     return false; 
    // }

    return goodNeighbour;

}

Entity RoomHandler::getFloor() const
{
	return this->floor;
}

const glm::vec3& RoomHandler::getRoomPos() const 
{
    return this->rooms[this->activeIndex].position;
}


void RoomHandler::createTileInfos(uint32_t roomIndex)
{
	auto rawTiles = this->rooms[roomIndex].mainTiles;
	auto& tileInfos = this->rooms[roomIndex].tileInfos;    

    const int NUM = 4;
    const glm::vec3 DIRS[NUM] =
	{
		{ 1,  0,  0 }, // Left
		{-1,  0,  0 }, // Right
		{ 0,  0,  1 }, // Down
		{ 0,  0,  -1}, // Up
	};

    for(size_t i = 0; i < rawTiles.size(); i++)
    {
        const glm::vec3 currentPos = rawTiles[i];        
        std::array<int,4> neighbours{TileInfo::NONE,TileInfo::NONE,TileInfo::NONE,TileInfo::NONE};
        
        for(size_t j = 0; j < rawTiles.size(); j++)
        {
            for(size_t k = 0; k < neighbours.size();k++) // Left, Right, Down, Up
            {
                if(neighbours[k] == TileInfo::NONE)
                {
                    //Distance between currentPos and possible neighbour pos
                    glm::vec3 sideOffset = currentPos + DIRS[k] * RoomHandler::TILE_WIDTH; 
                    sideOffset.y = 0.f;
                    float dist = glm::length(sideOffset - rawTiles[j]); 
                    
                    // Check if correct neighbour
                    if(dist < RoomHandler::TILE_WIDTH) 
                    {
                        neighbours[k] = j;
                    }
                    
                }
            }
        }
        tileInfos.emplace_back(currentPos, std::move(neighbours));
    }


#ifdef _CONSOLE
    TileInfo::checkValidTileInfoVector(tileInfos, roomIndex);
#endif
}

void RoomHandler::createDoors(int roomIndex, const glm::ivec2* doorTilePos)
{
	Room& curRoom = this->rooms[roomIndex];

	const float rots[4] = { -90.f, 90.f, 180.f, 0.f };

	const float OFFSET = 1.f;
	const glm::vec3 OFFSETS[4] =
	{
		glm::vec3(-OFFSET, 0.f, 0.f),
		glm::vec3(OFFSET, 0.f, 0.f),
		glm::vec3(0.f, 0.f, -OFFSET),
		glm::vec3(0.f, 0.f, OFFSET)
	};

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.connectingIndex[i] != -1)
		{
			curRoom.doors[i] = this->createDoorEntity(rots[i]);

			Transform& tra = this->scene->getComponent<Transform>(curRoom.doors[i]);
            this->scene->setComponent<EdgeTile>(curRoom.doors[i]);
			tra.position.x = ((float)doorTilePos[i].x + OFFSETS[i].x * 2.5f) * TILE_WIDTH;
			tra.position.z = ((float)doorTilePos[i].y + OFFSETS[i].z * 2.5f) * TILE_WIDTH;
			
			curRoom.objects.emplace_back(this->scene->createEntity());
			if(this->useMeshes)
            {
                this->scene->setComponent<MeshComponent>(curRoom.objects.back(), this->lampMeshId);
                MeshComponent& meshComp = this->scene->getComponent<MeshComponent>(curRoom.objects.back());
                this->resourceMan->makeUniqueMaterials(meshComp);
                meshComp.numOverrideMaterials = 1;
                meshComp.overrideMaterials[0].diffuseTextureIndex = this->lampDiffuseId;
                //meshComp.overrideMaterials[0].glowMapTextureIndex = this->lampGlowId;
                meshComp.overrideMaterials[0].emissionIntensity = 1.f;
            }

			Transform& doorTra = this->scene->getComponent<Transform>(curRoom.objects.back());
			doorTra.rotation = tra.rotation;
			doorTra.position = tra.position;
			doorTra.position += OFFSETS[i] * TILE_WIDTH;
			if (i <= 1)
			{
				doorTra.position.z += TILE_WIDTH * 0.5f * (i % 2 ? 1.f : -1.f);
			}
			else
			{
				doorTra.position.x += TILE_WIDTH * 0.5f * (i % 2 ? -1.f : 1.f);
			}
		}
	}
}

void RoomHandler::setConnections(int numMainRooms, const std::vector<glm::ivec2>& connections)
{
	this->exitPairs.resize(connections.size());
	this->verticalConnection.resize(connections.size());

	for (size_t i = 0; i < connections.size(); i++)
	{
		const glm::ivec2& curCon = connections[i];
		std::pair<glm::vec3, glm::vec3>& pair = this->exitPairs[i];

		if (curCon.x < numMainRooms && curCon.y < numMainRooms)
		{
			// Vertical Connection
			this->verticalConnection[i] = true;

			pair.first = this->roomExitPoints[curCon.y].positions[2];
			pair.second = this->roomExitPoints[curCon.x].positions[3];
		}
		else
		{
			// Horizontal connection
			this->verticalConnection[i] = false;

			pair.first = this->roomExitPoints[curCon.y].positions[0];
			pair.second = this->roomExitPoints[curCon.x].positions[1];
		}
	}
}

void RoomHandler::generatePathways()
{
	glm::vec3 delta{};
	glm::vec3 step{};
	glm::vec3 curPos{};
	Entity entity = -1;

	std::vector<glm::vec3> pathPositions;
	pathPositions.reserve(size_t(TILES_BETWEEN_ROOMS * 2u));

	// Will depend on where RoomGenerator places doors, but the while-loop has never gotten stuck yet tho
	const uint32_t maxIterations = TILES_BETWEEN_ROOMS * 4u;
	uint32_t numIterations = 0;

	// For each exit pair (the points which to make a path between)
	for (size_t i = 0; i < this->exitPairs.size(); i++)
	{
		glm::vec3& p0 = this->exitPairs[i].first;
		glm::vec3& p1 = this->exitPairs[i].second;

		if ((p1.x - p0.x) < 0.f)
		{
			std::swap(p0, p1);
		}

		delta = p1 - p0;
		delta.z = delta.z > 0.f ? -delta.z : delta.z;

		curPos = p0;

		step.x = p0.x < p1.x ? TILE_WIDTH : -TILE_WIDTH;
		step.z = p0.z < p1.z ? TILE_WIDTH : -TILE_WIDTH;

		float err = delta.x + delta.z;
		float e2 = 0.f;

		// Calcuate the path and save the positions
		numIterations = 0;
		while (numIterations++ < maxIterations)
		{
			pathPositions.emplace_back(curPos);

			if (glm::length(curPos - p0) >= glm::length(delta))
			{
				break;
			}

			e2 = TILE_WIDTH * 2.f * err;

			if (e2 > delta.z)
			{
				err += delta.z;
				curPos.x += step.x;
			}
			else if (e2 < delta.x)
			{
				err += delta.x;
				curPos.z += step.z;
			}
		}

		// Go through the path and generate a border around it
		const size_t startIdx = this->pathEntities.size();
		for (int l = 1; l <= 3; l++) // Thiccness off border around paths
		{
			this->surroundPaths(startIdx, pathPositions, p0, p1, (float)l, this->verticalConnection[i], l == 1);
		}

		for (const glm::vec3& pos : pathPositions)
		{
			if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
			{
				this->pathEntities.emplace_back(createFloorDecoEntity(glm::vec2(pos.x, pos.z), false));
			}
		}

		pathPositions.clear();
	}

	this->pathEntities.shrink_to_fit();
}

void RoomHandler::surroundPaths(size_t startIdx, const std::vector<glm::vec3>& pathPos, glm::vec3 p0, glm::vec3 p1, float distFactor, bool vertical, bool colliders)
{
	const glm::vec3 OFFSETS[] =
	{
		{TILE_WIDTH, 0, 0},
		{-TILE_WIDTH, 0, 0},
		{0, 0, TILE_WIDTH},
		{0, 0, -TILE_WIDTH},
	};
	const int NUM_OFFSETS = sizeof(OFFSETS) / sizeof(glm::vec3);

	const glm::vec3& upperP = p0.z > p1.z ? p0 : p1;
	const glm::vec3& lowerP = upperP == p0 ? p1 : p0;
	const glm::vec3& rightP = p0.x > p1.x ? p0 : p1;
	const glm::vec3& leftP = rightP == p0 ? p1 : p0;

	bool canPlace = true;
	for (size_t i = 0; i < pathPos.size(); i++)
	{
		const glm::vec3& pos = pathPos[i];

		for (int k = 0; k < NUM_OFFSETS; k++)
		{
			const glm::vec3 offsetPos = pos + OFFSETS[k] * distFactor;
			canPlace = true;

			// Don't place a border inside the room
			if (vertical)
			{
				canPlace = !(offsetPos.z > upperP.z || offsetPos.z < lowerP.z);
			}
			else
			{
				canPlace = !(offsetPos.x > rightP.x || offsetPos.x < leftP.x);
			}

			// Search through the path to see if offsetPos is on a tile
			for (size_t j = 0; j < pathPos.size() && canPlace; j++)
			{
				if (j != i)
				{
					glm::vec3 mPosToOffset = offsetPos - pathPos[j];
					mPosToOffset.y = 0.f;

					if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
					{
						canPlace = false;
					}
				}
			}

			// Search through the borders so no duplicates are made
			for (size_t j = startIdx; j < this->pathEntities.size() && canPlace; j++)
			{
				glm::vec3 mPosToOffset = offsetPos - this->scene->getComponent<Transform>(this->pathEntities[j]).position;
				mPosToOffset.y = 0.f;

				if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
				{
					canPlace = false;
				}
			}

			if (canPlace)
			{
				Entity entity = createBorderEntity({ offsetPos.x, offsetPos.z }, false);
				this->pathEntities.emplace_back(entity);

				if (colliders) // switch mesh and create collider for inner-layer borders
				{
					this->scene->setComponent<Collider>(entity, Collider::createBox(
						glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 3.f, TILE_WIDTH * 0.5f), glm::vec3(0.f, TILE_WIDTH * 3.f, 0.f)));

					if (this->useMeshes)
					{
						this->scene->getComponent<MeshComponent>(entity).meshID = this->innerBorderMesh;
					}
				}
			}
		}
	}
}

void RoomHandler::createFloor()
{
	float minX = 10000000.f;
	float maxX = -10000000.f;
	float minZ = 10000000.f;
	float maxZ = -10000000.f;
	float extents[4]{};

	for (Room& room : this->rooms)
	{
		if (room.position.x < minX) { minX = room.position.x; extents[LEFT_P] = room.extents[LEFT_P]; }
		if (room.position.x > maxX) { maxX = room.position.x; extents[RIGHT_P] = room.extents[RIGHT_P]; }
		if (room.position.z < minZ) { minZ = room.position.z; extents[UPPER_P] = room.extents[UPPER_P]; }
		if (room.position.z > maxZ) { maxZ = room.position.z; extents[LOWER_P] = room.extents[LOWER_P]; }
	}

	const glm::vec3 floorPos((minX + maxX) * 0.5f, 0.f, (minZ + maxZ) * 0.5f);
	const glm::vec3 floorDimensions((maxX - minX) * 0.5f + extents[LEFT_P] + extents[RIGHT_P], 4.f, (maxZ - minZ) * 0.5f + +extents[UPPER_P] + extents[LOWER_P]);

	this->floor = this->scene->createEntity();
	this->scene->setComponent<Collider>(this->floor, Collider::createBox(floorDimensions, glm::vec3(0.f, -4.f, 0.f)));
	this->scene->getComponent<Transform>(this->floor).position = floorPos;
	this->scene->getComponent<Transform>(this->floor).scale = glm::vec3(floorDimensions.x * 2.f, 1.f, floorDimensions.z * 2.f);
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(this->floor, (int)this->tileFloorMeshId);
	}
}

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;

	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->doorMeshID);
	}

	return entity;
}

Entity RoomHandler::createBorderEntity(const glm::vec2& position, bool scalePos)
{
	Entity entity = this->scene->createEntity();
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.position.x = position.x * (scalePos ? TILE_WIDTH : 1.f);
	transform.position.z = position.y * (scalePos ? TILE_WIDTH : 1.f);
	transform.rotation.y = 90.f * float(this->random->rand() % 4);
	
	if (this->useMeshes)
	{
        this->scene->setComponent<MeshComponent>(entity, (int)this->borderMeshIds[this->random->rand() % NUM_BORDER]);
	}
	else
	{
		this->random->rand();
	}

	return entity;
}

void RoomHandler::createObjectEntities(const Tile& tile, Room& room)
{
	Entity mainEntity = this->scene->createEntity();
	room.objects.emplace_back(mainEntity);

	Transform& transform = this->scene->getComponent<Transform>(mainEntity);
	transform.position = glm::vec3(tile.position.x, 0.f, tile.position.y);
	transform.position *= TILE_WIDTH;

	// Offset removed for now due to AI possible spawning too close and flying away
	//transform.position.x += float((int)this->random->rand() % 10 - 5); 
	//transform.position.z += float((int)this->random->rand() % 10 - 5);

	if (tile.type == Tile::TwoXOne || tile.type == Tile::OneXTwo)
	{
		if (tile.type == Tile::OneXTwo)
		{
			transform.rotation.y = 90.f;
		}

		transform.rotation.y += this->random->rand() % 2 ? 180.f : 0.f;
		transform.rotation.y += float((int)this->random->rand() % 10 - 5);
	}
	else if (tile.type == Tile::TwoXTwo)
	{
		transform.rotation.y = float((int)this->random->rand() % 360);
	}

	std::pair<int, int> pair(~0u, ~0u);

	if		(tile.type == Tile::TwoXTwo) { pair = this->twoXTwoMeshIds[this->random->rand() % NUM_TWO_X_TWO]; }
	else if (tile.type == Tile::TwoXOne) { pair = this->oneXTwoMeshIds[this->random->rand() % NUM_ONE_X_TWO]; }
	else if (tile.type == Tile::OneXTwo) { pair = this->oneXTwoMeshIds[this->random->rand() % NUM_ONE_X_TWO]; }

	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(mainEntity, pair.first);
	}
	
	if (pair.second != ~0u)
	{
		std::vector<ColliderDataRes> colliders = resourceMan->getCollisionShapeFromMesh(pair.second);
		Entity collEntity;
		for (size_t i = 0; i < colliders.size(); i++)
		{
			collEntity = this->scene->createEntity();
			this->scene->setComponent<Collider>(collEntity, colliders[i].col);
			Transform& collTra = this->scene->getComponent<Transform>(collEntity);
			collTra.position = transform.position + colliders[i].position;
			collTra.rotation = transform.rotation + colliders[i].rotation;

			room.objects.emplace_back(collEntity);
		}
	}
}

void RoomHandler::showPaths(bool show)
{
	if (show)
	{
		for (Entity tile : this->pathEntities)
		{
			this->scene->setActive(tile);
		}
	}
	else
	{
		for (Entity tile : this->pathEntities)
		{
			this->scene->setInactive(tile);
		}
	}
}

void RoomHandler::reset()
{
	for (Room& room : this->rooms)
	{
		for (const Entity& entity : room.objects)
		{
			this->scene->removeEntity(entity);
		}

		room.mainTiles.clear();
		room.objects.clear();

		for (int i = 0; i < 4; i++)
		{
			if (room.doors[i] != -1)
			{
				this->scene->removeEntity(room.doors[i]);
			}
			room.doors[i] = -1;
			room.connectingIndex[i] = -1;
			room.extents[i] = 0.f;
		}

		room.colliderPos = room.position = glm::vec3(0.f);
		room.finished = false;
		room.box.extents = glm::vec3(0.f);

		this->scene->removeEntity(room.rock);
		this->scene->removeEntity(room.rockFence);
	}
	this->rooms.clear();

	for (const Entity& entity : this->pathEntities)
	{
		this->scene->removeEntity(entity);
	}
	this->pathEntities.clear();

	this->scene->removeEntity(this->floor);
	this->floor = -1;

	this->exitPairs.clear();
	this->roomExitPoints.clear();

	this->activeIndex = 0;
}

void RoomHandler::closeDoors(int index)
{
#ifdef _CONSOLE
	if (index < 0 || index >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::closeDoors | Invalid index: " + std::to_string(index) + 
		". Num rooms: " + std::to_string(this->rooms.size()));
		return;
	}
#endif

	Room& room = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			this->scene->setScriptComponent(room.doors[i], "scripts/closedoor.lua");
			this->scene->setComponent<Collider>(room.doors[i], Collider::createBox(
					glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 2.f, TILE_WIDTH * 0.1f), glm::vec3(0.f, TILE_WIDTH * 2.f, 0.f)));
		}
	}
}

void RoomHandler::activateRoom(int index)
{
#ifdef _CONSOLE
	if (index < 0 || index >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::activateRoom | Invalid index: " + std::to_string(index) + 
		". Num rooms: " + std::to_string(this->rooms.size()));
		return;
	}
#endif

	Room& curRoom = this->rooms[index];
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setActive(entity);
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setActive(curRoom.doors[i]);
			this->scene->getComponent<Transform>(curRoom.doors[i]).position.y = -25;
		}
	}

	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->setActive(curRoom.rock);
		this->scene->setActive(curRoom.rockFence);
	}
}

void RoomHandler::deactivateRoom(int index)
{
#ifdef _CONSOLE
	if (index < 0 || index >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::deactivateRoom | Invalid index: " + std::to_string(index) + 
		". Num rooms: " + std::to_string(this->rooms.size()));
		return;
	}
#endif

	Room& curRoom = this->rooms[index];
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setInactive(entity);
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setInactive(curRoom.doors[i]);
		}
	}

	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->setInactive(curRoom.rock);
		this->scene->setInactive(curRoom.rockFence);
	}
}

void RoomHandler::placeDoorLamps()
{
	Room& curRoom = this->rooms[this->activeIndex];
	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			if (!this->rooms[curRoom.connectingIndex[i]].finished)
			{
				this->scene->setActive(this->doorLamps[i]);
				const Transform& doorTra = this->scene->getComponent<Transform>(curRoom.doors[i]);
				
				Transform& transform = this->scene->getComponent<Transform>(this->doorLamps[i]);
				transform.rotation = doorTra.rotation;
				transform.position.x = doorTra.position.x;
				transform.position.z = doorTra.position.z;
			}
		}
		else
		{
			this->scene->setInactive(this->doorLamps[i]);
		}
	}
}

#ifdef _CONSOLE
void RoomHandler::imgui(DebugRenderer* dr)
{
	if (ImGui::Begin("Rooms"))
	{
		if (ImGui::Button("Reload rooms only"))
		{
			this->generate(rand());
		}

		static bool drawExtents = false;
		static bool drawRooms = false;
		ImGui::Checkbox("extents", &drawExtents);
		ImGui::Checkbox("room box", &drawRooms);

		if (drawExtents)
		{
			glm::vec3 endPos;
			for (auto& room : rooms)
			{
				endPos = room.position + glm::vec3(room.extents[0], 0.f, 0.f);
				dr->renderLine(room.position, endPos, glm::vec3(1.f, 0.f, 0.f));

				endPos = room.position + glm::vec3(-room.extents[1], 0.f, 0.f);
				dr->renderLine(room.position, endPos, glm::vec3(1.f, 1.f, 0.f));

				endPos = room.position + glm::vec3(0.f, 0.f, room.extents[2]);
				dr->renderLine(room.position, endPos, glm::vec3(0.f, 0.f, 1.f));

				endPos = room.position + glm::vec3(0.f, 0.f, -room.extents[3]);
				dr->renderLine(room.position, endPos, glm::vec3(0.f, 1.f, 1.f));
			}
		}
		if (drawRooms)
		{
			for (Room& room : rooms  )
			{
				dr->renderBox(room.colliderPos, glm::vec3(0.f), room.box.extents * 2.f, glm::vec3(1.f, 0.f, 0.f));
			}
		}
	}
	ImGui::End();
}
#endif // _CONSOLE

std::vector<std::vector<glm::vec3>> RoomHandler::getPathFindingPoints() {
	return std::vector<std::vector<glm::vec3>>();
}