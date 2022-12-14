#include "Room Handler.h"
#include "vengine/application/Scene.hpp"
#include "vengine/dev/Random.hpp"
#include "vengine/physics/PhysicsEngine.h"
#include "vengine/application/Input.hpp"
#include "vengine/components/PointLight.hpp"
#include "vengine/network/NetworkHandler.h"

const float RoomHandler::TILE_WIDTH = 25.f;
const float RoomHandler::BORDER_COLLIDER_HEIGHT = TILE_WIDTH * 6.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 5;
const uint32_t RoomHandler::DECO_ENTITY_CHANCE = 30u;
const uint32_t RoomHandler::NUM_BORDER = 1;
const uint32_t RoomHandler::NUM_ONE_X_ONE = 5;
const uint32_t RoomHandler::NUM_ONE_X_TWO = 2;
const uint32_t RoomHandler::NUM_TWO_X_TWO = 2;

const glm::vec3 RoomHandler::DOOR_LAMP_OFFSET = glm::vec3(-12.f, 21.5f, 30.5f);
const glm::vec3 RoomHandler::DOOR_LAMP_COLOUR = glm::vec3(0.94f, 0.28f, 0.05f);
const float RoomHandler::DOOR_LAMP_INTENSITY = 5000.f;
const float RoomHandler::FLICKER_INTERVAL = 0.05f;
const int RoomHandler::FLICKER_INTENSITY = 60;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), activeIndex(0), 
	floor(-1), doorMeshID(0), tileFloorMeshId(0), innerBorderMesh(0),
	rockFenceMeshId(0), rockMeshId(0), random(), useMeshes(true),
	doorLamps{-1, -1, -1, -1}
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan,PhysicsEngine* physicsEngine, bool useMeshes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->physicsEngine = physicsEngine;
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
		this->lampMeshId = (int)resourceMan->addMesh("assets/models/Tiles/OneXTwo/lamp.obj");

		this->lampDiffuseId = (int)resourceMan->addTexture("assets/textures/lampTex.jpg");
		this->lampGlowId = (int)resourceMan->addTexture("assets/textures/Perk_HpTex.jpg");
	}
}

bool RoomHandler::playersOnCollider(Collider& col, const glm::vec3& pos, const std::vector<Entity>& players)
{
	int numCollisions = 0;
	
	for (size_t j = 0; j < players.size(); j++)
	{
		Collider& playerCol = this->scene->getComponent<Collider>(players[j]);
		const Transform& playerTra = this->scene->getComponent<Transform>(players[j]);
	
		if (this->physicsEngine->testContactPair(col, pos, glm::vec3(0.f), playerCol, playerTra.position, playerTra.rotation))
		{
			if (++numCollisions >= (int)players.size())
			{
				return true;
			}
		}
	}

	return false;
}

int RoomHandler::serverGetNextRoomIndex() const
{
	return this->serverNextIndex;
}

void RoomHandler::multiplayerToggleCurrentDoors(int nextIndex)
{
	this->toggleDoors(this->activeIndex, false);
	Room& oldRoom = this->rooms[this->activeIndex];
	for (int i = 0; i < 4; i++)
	{
		int conIdx = oldRoom.connectingIndex[i];
		if (conIdx != -1 && conIdx != nextIndex)
		{
			this->deactivateRoom(conIdx);
		}
	}

	Room& nextRoom = this->rooms[nextIndex];
	for (int i = 0; i < 4; i++)
	{
		int conIdx = nextRoom.connectingIndex[i];
		if (conIdx != -1 && conIdx != this->activeIndex)
		{
			this->activateRoom(conIdx);
		}
	}
	
	this->oldIndex = this->activeIndex;
	this->activeIndex = nextIndex;

	this->placeDoorLamps();
	this->togglePaths(this->oldIndex, false);
	this->togglePaths(this->activeIndex, true);
	this->toggleDoors(this->activeIndex, true);
}

void RoomHandler::mutliplayerCloseDoors()
{
	this->toggleDoors(this->oldIndex, true);
	this->toggleDoors(this->activeIndex, false);
	this->togglePaths(this->oldIndex, false);
	this->togglePaths(this->activeIndex, true);
	Room& curRoom = this->rooms[this->activeIndex];
	for (int i = 0; i < 4; i++)
	{
		int conIdx = curRoom.connectingIndex[i];
		if (conIdx != -1)
		{
			this->deactivateRoom(conIdx);
		}
	}
}

void RoomHandler::serverActivateCurrentRoom()
{
	Room& room = this->rooms[this->activeIndex];

	this->activateRoom(this->activeIndex);
	this->togglePaths(this->activeIndex, true);
	this->toggleDoors(this->oldIndex, false);

	for (int i = 0; i < 4; i++)
	{
		if (room.connectingIndex[i] != -1)
		{
			this->activateRoom(room.connectingIndex[i]);
		}
	}
}

void RoomHandler::serverDeactivateSurrounding()
{
	Room& room = this->rooms[this->activeIndex];
	this->toggleDoors(this->activeIndex, false);
	this->togglePaths(this->activeIndex, false);
	for (int i = 0; i < 4; i++)
	{
		if (room.connectingIndex[i] != -1)
		{
			this->deactivateRoom(room.connectingIndex[i]);
		}
	}
}

void RoomHandler::roomCompleted()
{	
	Room& curRoom = this->rooms[this->activeIndex];
	curRoom.finished = true;
	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->setScriptComponent(this->rooms[this->activeIndex].rock, "scripts/moveRock.lua");
	}

	if (this->useMeshes)
	{
		this->toggleDoors(this->activeIndex, true);
		if (this->oldIndex != -1)
		{
			this->toggleDoors(this->oldIndex, true);
		}

		for (int i = 0; i < 4; i++)
		{
			if (curRoom.connectingIndex[i] != -1)
			{
				this->activateRoom(curRoom.connectingIndex[i]);
			}
		}
	}
	else
	{
		this->deactivateRoom(this->activeIndex);
		this->togglePaths(this->activeIndex, false);
		for (int i = 0; i < 4; i++)
		{
			if (curRoom.connectingIndex[i] != -1)
			{
				this->deactivateRoom(curRoom.connectingIndex[i]);
			}
		}
	}
	
	this->oldIndex = -1;
	this->serverNextIndex = -1;
}

bool RoomHandler::playerNewRoom(Entity player)
{
	// Light flicker
	this->flickerTimer += Time::getDT();
	if (flickerTimer >= FLICKER_INTERVAL)
	{
		this->flickerTimer = 0.f;
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

	Collider& playerCol = this->scene->getComponent<Collider>(player);
	const Transform& playerTra = this->scene->getComponent<Transform>(player);

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			Room& newRoom = this->rooms[curRoom.connectingIndex[i]];

			if (this->physicsEngine->testContactPair(newRoom.box, newRoom.colliderPos, glm::vec3(0.f), 
				playerCol, playerTra.position, playerTra.rotation))
			{
				this->serverNextIndex = -1;
				this->oldIndex = this->activeIndex;
				this->activeIndex = curRoom.connectingIndex[i];
				this->placeDoorLamps();

				if (!newRoom.finished) // Room wasn't finished
				{
					for (int j = 0; j < 4; j++)
					{
						if (curRoom.connectingIndex[j] != -1 && curRoom.connectingIndex[j] != this->activeIndex)
						{
							this->deactivateRoom(curRoom.connectingIndex[j]);
						}
						if (newRoom.connectingIndex[j] == this->oldIndex)
						{
							this->respawnDoorIdx = j;
						}
					}

					NetworkHandler* network = this->scene->getNetworkHandler();
					if (network)
					{
						if (!network->isConnected())
						{
							this->deactivateRoom(this->oldIndex);
							this->toggleDoors(this->oldIndex, true);
							this->togglePaths(this->oldIndex, false);

							this->toggleDoors(this->activeIndex, false);
							this->togglePaths(this->activeIndex, true);
						}
					}
					
					return true;
				}

				// Room was finished
				for (int j = 0; j < 4; j++)
				{
					if (curRoom.doors[j] != -1 && curRoom.connectingIndex[j] != this->activeIndex)
					{
						this->deactivateRoom(curRoom.connectingIndex[j]);
					}
					if (newRoom.doors[j] != -1 && newRoom.connectingIndex[j] != oldIndex)
					{
						this->activateRoom(newRoom.connectingIndex[j]);
					}
				}
				
				this->togglePaths(this->oldIndex, false);
				this->togglePaths(this->activeIndex, true);

				return false;
			}
		}
	}
	
	return false;
}

bool RoomHandler::playersInPathway(const std::vector<Entity>& players)
{
	Room& curRoom = this->rooms[this->activeIndex];

	// Find player 1
	Collider& p1Col = this->scene->getComponent<Collider>(players[0]);
	Transform& p1Tra = this->scene->getComponent<Transform>(players[0]);

	for (int i = 0; i < (int)this->rooms.size(); i++)
	{
		if (this->physicsEngine->testContactPair(
			this->rooms[i].box, this->rooms[i].colliderPos, glm::vec3(0.f), p1Col, p1Tra.position, p1Tra.rotation))
		{
			this->activeIndex = i;
			return false;
		}
	}

	// For all connecting paths..
	for (int i = 0; i < 4; i++)
	{
		if (curRoom.connPathIndex[i] != -1)
		{
			if (!this->rooms[curRoom.connectingIndex[i]].finished)
			{
				// ..Check if all players are inside the path
				Pathway& path = this->paths[curRoom.connPathIndex[i]];
				if (this->playersOnCollider(path.box, path.colliderPos, players))
				{
					this->oldIndex = this->activeIndex;
					this->activeIndex = curRoom.connectingIndex[i];
					this->serverNextIndex = curRoom.connectingIndex[i];
					return true;
				}
			}
		}
	}

	return false;
}

bool RoomHandler::playersInsideNewRoom(const std::vector<Entity>& players)
{
	Room& curRoom = this->rooms[this->activeIndex];
	return this->playersOnCollider(curRoom.box, curRoom.colliderPos, players);
}

void RoomHandler::startOver()
{
	if (this->activeIndex == -1 || this->oldIndex == -1)
	{
		return;
	}
	Room& failedRoom = this->rooms[this->activeIndex];
	Room& prevRoom = this->rooms[this->oldIndex];

	for (int i = 0; i < 4; i++)
	{
		if (failedRoom.doors[i] != -1)
		{
			this->scene->setScriptComponent(failedRoom.doors[i], "scripts/opendoor.lua");
			this->scene->removeComponent<Collider>(failedRoom.doors[i]);
		}
		if (prevRoom.doors[i] != -1 && prevRoom.connectingIndex[i] != this->activeIndex)
		{
			this->activateRoom(prevRoom.connectingIndex[i]);
		}
	}
	this->activateRoom(this->oldIndex);
	this->togglePaths(this->oldIndex, true);
	this->activeIndex = this->oldIndex;
	this->oldIndex = -1;

	this->placeDoorLamps();
}

glm::vec3 RoomHandler::getRespawnPos() const
{
	if (this->oldIndex == -1)
	{
		return glm::vec3(0.0f);
	}
	const glm::vec3 respawnOffset[] =
	{
		glm::vec3(TILE_WIDTH, 0.f, 0.f),
		glm::vec3(-TILE_WIDTH, 0.f, 0.f),
		glm::vec3(0.f, 0.f, TILE_WIDTH),
		glm::vec3(0.f, 0.f, -TILE_WIDTH)
	};

	Entity door = this->rooms[this->activeIndex].doors[this->respawnDoorIdx];
	glm::vec3 pos = this->scene->getComponent<Transform>(door).position + respawnOffset[this->respawnDoorIdx];
	pos.y = 12.f;
	return pos;
}

glm::vec3 RoomHandler::getRespawnRot() const
{
	if (this->respawnDoorIdx == -1)
	{
		return glm::vec3(0.0f);
	}
	const float respawnYRot[] =
	{
		-90.f,
		90.f,
		180.f,
		0.f
	};
	return glm::vec3(0.f, respawnYRot[this->respawnDoorIdx], 0.f);
}

void RoomHandler::generate(uint32_t seed, uint16_t level)
{
	const glm::vec3 noDoorBoxOffset[] = 
	{ 
		glm::vec3(TILE_WIDTH, 0.f, 0.f),
		glm::vec3(-TILE_WIDTH, 0.f, 0.f),
		glm::vec3(0.f, 0.f, TILE_WIDTH),
		glm::vec3(0.f, 0.f, -TILE_WIDTH)
	};

	this->random = new VRandom;
	this->random->srand(seed);

	RoomLayout roomLayout(*this->random);
	RoomGenerator roomGen(*this->random);
	roomGen.setDesc(this->getRoomDesc(level));

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
				if (std::abs(int(this->random->rand())) % 100 < DECO_ENTITY_CHANCE)
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
				if (std::abs(int(this->random->rand())) % 100 < DECO_ENTITY_CHANCE)
				{
					curRoom.objects.emplace_back(this->createFloorDecoEntity(tile.position, true));
				}
				break;

			case Tile::AI:
				if (std::abs(int(this->random->rand())) % 100 < DECO_ENTITY_CHANCE)
				{
					curRoom.objects.emplace_back(this->createFloorDecoEntity(tile.position, true));
				}

#if 0 // Show AI tiles
				entity = scene->createEntity();
				curRoom.objects.emplace_back(entity);
				if (this->useMeshes)
				{
					this->scene->setComponent<MeshComponent>(entity, this->twoXTwoMeshIds[0].first);
					MeshComponent& meshComp = this->scene->getComponent<MeshComponent>(entity);
				}
				Transform& tra = this->scene->getComponent<Transform>(entity);
				tra.position.x = tile.position.x * TILE_WIDTH;
				tra.position.z = tile.position.y * TILE_WIDTH;
				tra.scale *= 0.25f;
#endif

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
			this->scene->setComponent<RockFenceComp>(entity);
			this->scene->setComponent<Collider>(entity, Collider::createBox(
					glm::vec3(TILE_WIDTH, TILE_WIDTH * 2.f, TILE_WIDTH), glm::vec3(0.f, TILE_WIDTH, 0.f)));
			if (this->useMeshes)
			{
				this->scene->setComponent<MeshComponent>(entity, (int)this->rockFenceMeshId);
			}

			entity = this->scene->createEntity();
			this->scene->setComponent<Collider>(entity, Collider::createCapsule(8.f, 200.f));
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
		else if (curRoom.type == RoomData::EXIT_ROOM)
		{
			this->portalRoomIndex = i;
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
	for (Pathway& path : this->paths)
	{
		for (Entity entity : path.entities)
		{
			this->scene->getComponent<Transform>(entity).position += glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f);
		}
	}
	// Keep for testing
	//printf("Num Rooms: %zd | numPathsEntities: %zd\n", rooms.size(), pathEntities.size());
	for (int i = 0; i < numTotRooms; i++)
	{
		this->moveRoom(i, glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f));

		this->rooms[i].mainTiles.shrink_to_fit();
		this->rooms[i].objects.shrink_to_fit();

		// Keep for testing
		//printf("Room: %d\n", i);
		//printf("Num mainTiles: %zd | numObjects: %zd\n", rooms[i].mainTiles.size(), rooms[i].objects.size());
		//printf("Connecting idx: 0: %d, 1: %d, 2: %d, 3: %d\n", rooms[i].connectingIndex[0], rooms[i].connectingIndex[1], rooms[i].connectingIndex[2], rooms[i].connectingIndex[3]);
		//printf("Type: %d | pos: (%d, %d, %d)\n----------\n", (int)rooms[i].type, (int)rooms[i].position.x, (int)rooms[i].position.y, (int)rooms[i].position.z);

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
	for (int i = 1; i < numTotRooms; i++)
	{
		this->deactivateRoom(i);	
		this->togglePaths(i, false);
	}

	this->activeIndex = 0;
	this->placeDoorLamps();
	this->togglePaths(this->activeIndex, true);
	Room& startRoom = this->rooms[this->activeIndex];
	startRoom.finished = true;

	NetworkHandler* network = this->scene->getNetworkHandler();
	if (network)
	{
		if (network->isConnected())
		{
			for (int i = 1; i < (int)this->rooms.size(); i++)
			{
				this->forceToggleDoors(i, false);
				this->deactivateRoom(i);
			}

			for (int i = 0; i < 4; i++)
			{
				if (startRoom.doors[i] != -1)
				{
					this->scene->setScriptComponent(startRoom.doors[i], "scripts/opendoor.lua");
					this->activateRoom(startRoom.connectingIndex[i]);
				}
			}
		}
		else
		{ 
			for (size_t i = 0; i < this->rooms.size(); i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (this->rooms[i].doors[j] != -1)
					{
						this->scene->setScriptComponent(this->rooms[i].doors[j], "scripts/opendoor.lua");
						if (i == 0)
						{
							this->activateRoom(startRoom.connectingIndex[j]);
						}
					}
				}
			}
		}
	}
	
	// Server side
	if (!this->useMeshes)
	{
		for (size_t i = 0; i < this->rooms.size(); i++)
		{
			this->deactivateRoom(i);
		}
		for (size_t i = 0; i < this->paths.size(); i++)
		{
			this->togglePaths(i, false);
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

RoomGenerator::RoomDescription RoomHandler::getRoomDesc(uint16_t level)
{
	RoomGenerator::RoomDescription desc;

#ifdef _CONSOLE
	if (this->overrideLevel)
	{
		return this->desc;
	}
#endif

	switch (level)
	{
	default:
		break;

	case 0u:
		desc.radius = 3u;
		
		desc.numBranches = 3u;
		desc.branchDepth = 1u;
		desc.branchDist = 3u;

		desc.twoXTwoChance = 0u;
		desc.maxTwoXTwo = 0u;
		desc.oneXTwoChance = 100u;
		desc.maxOneXTwo = 1u;

		desc.maxAngle = 45u;

		break;

	case 1u:
		desc.radius = 3u;
		
		desc.numBranches = 4u;
		desc.branchDepth = 2u;
		desc.branchDist = 3u;

		desc.twoXTwoChance = 50u;
		desc.maxTwoXTwo = 1u;
		desc.oneXTwoChance = 50u;
		desc.maxOneXTwo = 2u;

		desc.maxAngle = 45u;
		break;
	}

	return desc;
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
	transform.position.x = pos.x * (scalePos ? TILE_WIDTH : 1.f) + float(std::abs(int(this->random->rand())) % 8 - 4);
	transform.position.z = pos.y * (scalePos ? TILE_WIDTH : 1.f) + float(std::abs(int(this->random->rand())) % 8 - 4);
	transform.rotation.y = float(std::abs(int(this->random->rand())) % 360);
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->oneXOneMeshIds[std::abs(int(this->random->rand())) % NUM_ONE_X_ONE]);
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
	return this->rooms[this->portalRoomIndex];
}

bool RoomHandler::isPortalRoomDone() const
{
	return this->rooms[this->portalRoomIndex].finished;
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
    for(int i = 0; i < (int)tileInfos.size(); i++ )
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

int RoomHandler::getActiveIndex() const
{
	return this->activeIndex;
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
			
			// Lamp mesh by doors
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
	this->paths.resize(connections.size());
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

			pair.first = this->roomExitPoints[curCon.y].positions[UPPER_P];
			pair.second = this->roomExitPoints[curCon.x].positions[LOWER_P];	
			
			this->rooms[curCon.y].connPathIndex[UPPER_P] = (int)i;
			this->rooms[curCon.x].connPathIndex[LOWER_P] = (int)i;
		}
		else
		{
			// Horizontal connection
			this->verticalConnection[i] = false;

			pair.first = this->roomExitPoints[curCon.y].positions[LEFT_P];
			pair.second = this->roomExitPoints[curCon.x].positions[RIGHT_P];

			this->rooms[curCon.y].connPathIndex[LEFT_P] = (int)i;
			this->rooms[curCon.x].connPathIndex[RIGHT_P] = (int)i;
		}

		glm::vec3 delta = glm::abs(pair.first - pair.second);
		if (delta.x < 1.f) { delta.x += TILE_WIDTH;}
		if (delta.z < 1.f) { delta.z += TILE_WIDTH;}

		this->paths[i].box = Collider::createBox(delta);
		this->paths[i].box.extents.y = TILE_WIDTH;

		this->paths[i].colliderPos = (pair.first + pair.second) * 0.5f;
		this->paths[i].colliderPos.y = TILE_WIDTH;

		this->paths[i].colliderPos.x -= TILE_WIDTH * 0.5f;
		this->paths[i].colliderPos.z -= TILE_WIDTH * 0.5f;
		if (this->verticalConnection[i])
		{ 
			this->paths[i].box.extents.z -= TILE_WIDTH * 0.5f;
			this->paths[i].colliderPos.z += TILE_WIDTH * 0.4f;
		}
		else
		{ 
			this->paths[i].box.extents.x -= TILE_WIDTH * 0.5f;
			this->paths[i].colliderPos.x += TILE_WIDTH * 0.4f * (this->paths[i].colliderPos.x > 0.f ? 1.f : -1.f);
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
		for (int l = 1; l <= 3; l++) // Thiccness off border around paths
		{
			this->surroundPaths(i, pathPositions, p0, p1, (float)l, this->verticalConnection[i], l == 1);
		}

		for (const glm::vec3& pos : pathPositions)
		{
			if (std::abs(int(this->random->rand())) % 100 < DECO_ENTITY_CHANCE)
			{
				this->paths[i].entities.emplace_back(createFloorDecoEntity(glm::vec2(pos.x, pos.z), false));
			}
		}

		pathPositions.clear();
		this->paths[i].entities.shrink_to_fit();
	}
}

void RoomHandler::surroundPaths(size_t pathIndex, const std::vector<glm::vec3>& pathPos, glm::vec3 p0, glm::vec3 p1, float distFactor, bool vertical, bool colliders)
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
			for (size_t j = 0; j < this->paths[pathIndex].entities.size() && canPlace; j++)
			{
				glm::vec3 mPosToOffset = offsetPos - this->scene->getComponent<Transform>(this->paths[pathIndex].entities[j]).position;
				mPosToOffset.y = 0.f;

				if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
				{
					canPlace = false;
				}
			}

			if (canPlace)
			{
				Entity entity = createBorderEntity({ offsetPos.x, offsetPos.z }, false);
				this->paths[pathIndex].entities.emplace_back(entity);

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
	transform.rotation.y = 90.f * float(std::abs(int(this->random->rand())) % 4);
	
	if (this->useMeshes)
	{
        this->scene->setComponent<MeshComponent>(entity, (int)this->borderMeshIds[std::abs(int(this->random->rand())) % NUM_BORDER]);
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
	//transform.position.x += float(std::abs(int(this->random->rand())) % 10 - 5); 
	//transform.position.z += float(std::abs(int(this->random->rand())) % 10 - 5);

	if (tile.type == Tile::TwoXOne || tile.type == Tile::OneXTwo)
	{
		if (tile.type == Tile::OneXTwo)
		{
			transform.rotation.y = 90.f;
		}

		transform.rotation.y += std::abs(int(this->random->rand())) % 2 ? 180.f : 0.f;
		transform.rotation.y += float(std::abs(int(this->random->rand())) % 10 - 5);
	}
	else if (tile.type == Tile::TwoXTwo)
	{
		transform.rotation.y = float(std::abs(int(this->random->rand())) % 30 + (std::abs(int(this->random->rand())) % 4) * 90);
	}

	std::pair<int, int> pair(~0u, ~0u);

	if		(tile.type == Tile::TwoXTwo) { pair = this->twoXTwoMeshIds[std::abs(int(this->random->rand())) % NUM_TWO_X_TWO]; }
	else if (tile.type == Tile::TwoXOne) { pair = this->oneXTwoMeshIds[std::abs(int(this->random->rand())) % NUM_ONE_X_TWO]; }
	else if (tile.type == Tile::OneXTwo) { pair = this->oneXTwoMeshIds[std::abs(int(this->random->rand())) % NUM_ONE_X_TWO]; }

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

void RoomHandler::togglePaths(int roomIndex, bool show)
{
#ifdef _CONSOLE
	if (roomIndex < 0 || roomIndex >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::showPaths | Invalid roomIndex: " + std::to_string(roomIndex)
			+ ". Num rooms: " + std::to_string(this->rooms.size()));

		return;
	}
#endif
	Room& curRoom = this->rooms[roomIndex];
	for (int i = 0; i < 4; i++)
	{
		const int pathIdx = curRoom.connPathIndex[i];
		if (pathIdx != -1)
		{
			if (show)
			{
				for (Entity entity : this->paths[pathIdx].entities)
				{
					this->scene->setActive(entity);
				}
			}
			else
			{
				for (Entity entity : this->paths[pathIdx].entities)
				{
					this->scene->setInactive(entity);
				}
			}
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
		}

		this->scene->removeEntity(room.rock);
		this->scene->removeEntity(room.rockFence);
	}
	this->rooms.clear();

	for (Pathway& path : this->paths)
	{
		for (const Entity& entity : path.entities)
		{
			this->scene->removeEntity(entity);
		}
		path.entities.clear();
	}
	this->paths.clear();

	for (int i = 0; i < 4; i++)
	{	
		if (doorLamps[i] != -1)
		{
			this->scene->removeEntity(this->doorLamps[i]);
		}

		this->doorLamps[i] = -1;
	}

	this->scene->removeEntity(this->floor);
	this->floor = -1;

	this->exitPairs.clear();
	this->roomExitPoints.clear();

	this->activeIndex = 0;
	this->serverNextIndex = -1;
	this->prevRoomIndex = -1;
	this->oldIndex = -1;
	this->respawnDoorIdx = -1;
}

void RoomHandler::toggleDoors(int index, bool open, int ignore)
{
#ifdef _CONSOLE
	if (index < 0 || index >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::toggleDoors | Invalid index: " + std::to_string(index) + 
		". Num rooms: " + std::to_string(this->rooms.size()));
		return;
	}
#endif

	Room& room = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			if (open/* && this->scene->hasComponents<Collider>(room.doors[i])*/)
			{
				this->scene->setScriptComponent(room.doors[i], "scripts/opendoor.lua");
				this->scene->removeComponent<Collider>(room.doors[i]);
			}
			else// if (!this->scene->hasComponents<Collider>(room.doors[i]))
			{
				this->scene->setScriptComponent(room.doors[i], "scripts/closedoor.lua");
				this->scene->setComponent<Collider>(room.doors[i], Collider::createBox(
						glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 2.f, TILE_WIDTH * 0.1f), glm::vec3(0.f, TILE_WIDTH * 2.f, 0.f)));
			}
		}
	}
}

void RoomHandler::forceToggleDoors(int index, bool open, int ignore)
{
	#ifdef _CONSOLE
	if (index < 0 || index >= (int)this->rooms.size())
	{
		Log::error("RoomHandler::forceToggleDoors | Invalid index: " + std::to_string(index) + 
		". Num rooms: " + std::to_string(this->rooms.size()));
		return;
	}
#endif

	Room& room = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			if (open/* && this->scene->hasComponents<Collider>(room.doors[i])*/)
			{
				this->scene->getComponent<Transform>(room.doors[i]).position.y = -25.f;
				this->scene->removeComponent<Collider>(room.doors[i]);
			}
			else// if (!this->scene->hasComponents<Collider>(room.doors[i]))
			{
				this->scene->getComponent<Transform>(room.doors[i]).position.y = 0.f;
				this->scene->setComponent<Collider>(room.doors[i], Collider::createBox(
						glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 2.f, TILE_WIDTH * 0.1f), glm::vec3(0.f, TILE_WIDTH * 2.f, 0.f)));
			}
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

const std::vector<RoomHandler::Room>& RoomHandler::getRooms() const
{
	return this->rooms;
}

const std::vector<RoomHandler::Pathway>& RoomHandler::getPaths() const
{
	return this->paths;
}

#ifdef _CONSOLE
#include "../Scenes/RoomTesting.h"
#include "vengine/graphics/DebugRenderer.hpp"

#define IMGUI_INT_SLIDER(X)\
ImGui::InputInt(#X, &X, 1, 4);\
X = glm::clamp(X, 0, 1000);\
this->desc.X = X

void RoomHandler::imgui(DebugRenderer* dr)
{
	if (ImGui::Begin("Rooms"))
	{
		ImGui::PushItemWidth(-120.f);
		if (dynamic_cast<RoomTesting*>(this->scene))
		{
			static int level = 0;

			ImGui::Checkbox("Override level", &this->overrideLevel);

			static int twoXTwoChance = 20u;
			static int maxTwoXTwo = 6u;
			static int oneXTwoChance = 20u;
			static int maxOneXTwo = 6u;
			static int bigTileMinDist = 1u;
			static int radius = 4u;
			static int numBranches = 3u;
			static int branchDepth = 3u;
			static int branchDist = 3u;
			static int maxAngle = 60u;

			if (this->overrideLevel)
			{
				
				IMGUI_INT_SLIDER(twoXTwoChance);
				IMGUI_INT_SLIDER(maxTwoXTwo);
				IMGUI_INT_SLIDER(oneXTwoChance);
				IMGUI_INT_SLIDER(maxOneXTwo);
				IMGUI_INT_SLIDER(bigTileMinDist);
				IMGUI_INT_SLIDER(radius);
				IMGUI_INT_SLIDER(numBranches);
				IMGUI_INT_SLIDER(branchDepth);
				IMGUI_INT_SLIDER(branchDist);
				IMGUI_INT_SLIDER(maxAngle);

				if (ImGui::Button("Reset values"))
				{
					twoXTwoChance = 20u;
					maxTwoXTwo = 6u;
					oneXTwoChance = 20u;
					maxOneXTwo = 6u;
					bigTileMinDist = 1u;
					radius = 4u;
					numBranches = 3u;
					branchDepth = 3u;
					branchDist = 3u;
					maxAngle = 60u;
				}
			}
			else
			{
				ImGui::InputInt("Level", &level, 1, 1);
				glm::clamp(level, 0, 100);
			}


			if (ImGui::Button("Reload rooms"))
			{
				if (!this->overrideLevel)
				{
					this->desc = getRoomDesc((uint16_t)level);
					twoXTwoChance = desc.twoXTwoChance;
					maxTwoXTwo = desc.maxTwoXTwo;
					oneXTwoChance = desc.oneXTwoChance;
					maxOneXTwo = desc.maxOneXTwo;
					bigTileMinDist = desc.bigTileMinDist;
					radius = desc.radius;
					numBranches = desc.numBranches;
					branchDepth = desc.branchDepth;
					branchDist = desc.branchDist;
					maxAngle = desc.maxAngle;
				}
				int seed = rand();
				printf("Seed: %d\n", seed);
				this->generate((uint32_t)seed, level);
			}

			if (ImGui::Button("Complete room"))
			{
				this->roomCompleted();
			}
		}

		static bool drawExtents = false;
		static bool drawRooms = false;
		static bool drawPathBox = false;
		ImGui::Text("Active: %d", activeIndex);
		ImGui::Checkbox("extents", &drawExtents);
		ImGui::Checkbox("room box", &drawRooms);
		ImGui::Checkbox("path box", &drawPathBox);

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
		if (drawPathBox)
		{
			for (Pathway& path : paths)
			{
				dr->renderBox(path.colliderPos, glm::vec3(0.f), path.box.extents * 2.f, glm::vec3(0.6f, 0.f, 0.4f));
			}
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();
}
#endif // _CONSOLE

std::vector<std::vector<glm::vec3>> RoomHandler::getPathFindingPoints() {
	return std::vector<std::vector<glm::vec3>>();
}