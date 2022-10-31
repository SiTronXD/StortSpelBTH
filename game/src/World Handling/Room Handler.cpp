#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::TILE_WIDTH = 25.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 3;
const uint32_t RoomHandler::NUM_BORDER = 1;
const uint32_t RoomHandler::NUM_ONE_X_ONE = 4;
const uint32_t RoomHandler::NUM_ONE_X_TWO = 0;
const uint32_t RoomHandler::NUM_TWO_X_TWO = 0;

RoomHandler::RoomHandler()
	:scene(nullptr), hasDoor{false, false, false, false},
	activeIndex(0), nextIndex(-1), floor(-1),
	doorMeshID(0)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes)
{
	roomSize = 15;

	this->scene = scene;

	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.setRoomDistance(TILE_WIDTH * roomSize + TILE_WIDTH * TILES_BETWEEN_ROOMS);	

	this->borderMeshIds.resize(NUM_BORDER);
	for (uint32_t i = 0; i < NUM_BORDER; i++)
	{
		this->borderMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/Border/" + std::to_string(i + 1u) + ".obj");
	}

	this->oneXOneMeshIds.resize(NUM_ONE_X_ONE);
	for (uint32_t i = 0; i < NUM_ONE_X_ONE; i++)
	{
		this->oneXOneMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/OneXOne/" + std::to_string(i + 1u) + ".obj");
	}

	// OneXTwo
	// TwoXTwo

	this->doorMeshID = resourceMan->addMesh("assets/models/door.obj");
}

void RoomHandler::roomCompleted()
{
	this->rooms[this->activeIndex].finished = true;

	this->openDoors(this->activeIndex);
	this->showPaths(true);
}

bool RoomHandler::onPlayerTrigger(Entity otherEntity)
{
	if (this->checkRoom(this->activeIndex, otherEntity))
	{
		return true;
	}

	if (this->nextIndex != -1)
	{ 
		if (this->checkRoom(this->nextIndex, otherEntity))
		{
			return true;
		}
	}

	return false;
}

void RoomHandler::generate()
{
	this->reset();

	this->roomLayout.generate();
	const int numRooms = this->roomLayout.getNumRooms();
	this->rooms.resize(numRooms);

	for (int i = 0; i < numRooms; i++)
	{
		const RoomData& roomRef = this->roomLayout.getRoom(i);

		this->rooms[i].position = roomRef.position;
		this->rooms[i].type = roomRef.type;

		// Save connecting indices
		this->rooms[i].connectingIndex[0] = roomRef.left;
		this->rooms[i].connectingIndex[1] = roomRef.right;
		this->rooms[i].connectingIndex[2] = roomRef.up;
		this->rooms[i].connectingIndex[3] = roomRef.down;

		// Generate room (no borders)
		this->roomGenerator.generateRoom();

		this->rooms[i].tiles.reserve(size_t(this->roomGenerator.getNrTiles()));
		for (int j = 0; j < this->roomGenerator.getNrTiles(); j++) 
		{
			this->rooms[i].tiles.emplace_back(this->createTileEntity(j, TileUsage::Default));
		}

		this->createDoors(i);
		this->roomGenerator.generateBorders(this->hasDoor);

		// Save room exit points
		this->setExitPoints(i);

		this->rooms[i].borders.reserve(size_t(this->roomGenerator.getNrBorders()));
		for (int j = 0; j < this->roomGenerator.getNrBorders(); j++) 
		{
			this->rooms[i].borders.emplace_back(this->createTileEntity(j, TileUsage::Border));
		}

		// Save exit paths
		this->rooms[i].exitPaths.reserve(size_t(this->roomGenerator.getNrExitTiles()));
		for (int j = 0; j < this->roomGenerator.getNrExitTiles(); j++) 
		{
			this->rooms[i].exitPaths.emplace_back(this->createTileEntity(j, TileUsage::Exit));
		}

		// Transform the room to worldspace (scale and move)
		// RoomGenerator is dependant on a tile being 1x1
		this->roomToWorldSpace(i);
		
		// Reset the generator
		this->roomGenerator.reset();
	}

	this->setConnections();
	this->generatePathways();
	this->createColliders();
	
	this->roomLayout.clear();
	this->exitPairs.clear();
	this->roomExitPoints.clear();
	this->verticalConnection.clear();

#ifdef _CONSOLE
	if (this->showAllRooms) 
	{ 
		for (int i = 0; i < (int)this->rooms.size(); i++)
		{
			this->activateRoom(i);
		}
		this->showPaths(true);
	}
	else
	{
		for (int i = 0; i < (int)this->rooms.size(); i++)
		{
			if (i != this->activeIndex) 
			{ 
				this->deactivateRoom(i); 
			}
		}
		this->showPaths(true);
	}
#else
	for (int i = 0; i < (int)this->rooms.size(); i++)
	{
		if (i != this->activeIndex)
		{
			this->deactivateRoom(i);
		}
	}
	this->showPaths(true);

#endif // _CONSOLE

	this->rooms[this->activeIndex].finished = true;
	for (int i = 0; i < 4; i++)
	{
		if (this->rooms[this->activeIndex].doors[i] != -1)
		{
			this->scene->setScriptComponent(this->rooms[this->activeIndex].doors[i], "scripts/opendoor.lua");
		}
	}
}

const std::vector<Entity>& RoomHandler::getFreeTiles()
{
	return this->rooms[this->activeIndex].tiles;
}

const RoomData::Type& RoomHandler::getActiveRoomType() const
{
	return this->rooms[this->activeIndex].type;
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

void RoomHandler::createDoors(int roomIndex)
{
	const RoomData& curRoom = this->roomLayout.getRoom(roomIndex);
	const glm::vec3& roomPos = curRoom.position;

	Room& curRoomIds = this->rooms[roomIndex];

	if (curRoom.left  != -1) { curRoomIds.doors[0] = this->createDoorEntity(-90.f); }
	if (curRoom.right != -1) { curRoomIds.doors[1] = this->createDoorEntity(90.f);  }
	if (curRoom.up	  != -1) { curRoomIds.doors[2] = this->createDoorEntity(180.f); }
	if (curRoom.down  != -1) { curRoomIds.doors[3] = this->createDoorEntity(0.f);	  }

	const float OFFSET = 1.5f;
	const glm::vec2 OFFSETS[4] = 
	{
		glm::vec2(OFFSET, 0.f),
		glm::vec2(-OFFSET, 0.f),
		glm::vec2(0.f, OFFSET),
		glm::vec2(0.f, -OFFSET)
	};

	const glm::vec2* doorTilePos = this->roomGenerator.getMinMaxPos();
	for (int i = 0; i < 4; i++)
	{
		this->hasDoor[i] = false;

		if (curRoomIds.doors[i] != -1)
		{
			this->hasDoor[i] = true;
			Transform& tra = this->scene->getComponent<Transform>(curRoomIds.doors[i]);

			// offset by 1.5 (1.5 tiles before room scaling)
			tra.position.x = doorTilePos[i].x + OFFSETS[i].x;
			tra.position.z = doorTilePos[i].y + OFFSETS[i].y;
			tra.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);
		}
	}
}

void RoomHandler::setConnections()
{
	const int NUM_MAIN_ROOMS = this->roomLayout.getNumMainRooms();
	const std::vector<glm::ivec2>& connections = this->roomLayout.getConnections();

	this->exitPairs.resize(connections.size());
	this->verticalConnection.resize(connections.size());

	for (size_t i = 0; i < connections.size(); i++)
	{
		const glm::ivec2& curCon = connections[i];
		std::pair<glm::vec3, glm::vec3>& pair = exitPairs[i];

		if (curCon.x < NUM_MAIN_ROOMS && curCon.y < NUM_MAIN_ROOMS)
		{
			// Vertical Connection
			// curCon.x = index of upper room
			// curCon.y = index of lower room

			this->verticalConnection[i] = true;

			pair.first = this->roomExitPoints[curCon.y].positions[2];
			pair.first.z += TILE_WIDTH;

			pair.second = this->roomExitPoints[curCon.x].positions[3];
			pair.second.z -= TILE_WIDTH;
		}
		else
		{
			// Horizontal connection
			// curCon.x = right room
			// curCon.y = left room

			this->verticalConnection[i] = false;

			pair.first = this->roomExitPoints[curCon.y].positions[0];
			pair.first.x += TILE_WIDTH;

			pair.second = this->roomExitPoints[curCon.x].positions[1];
			pair.second.x -= TILE_WIDTH;
		}
	}
}

void RoomHandler::setExitPoints(int roomIndex)
{
	const glm::vec2* exitPositions = this->roomGenerator.getExitTilesPos();

	this->roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (this->rooms[roomIndex].doors[i] != -1)
		{ 
			this->roomExitPoints.back().positions[i] = glm::vec3(exitPositions[i].x, 0.f, exitPositions[i].y);
		}
	}
}

void RoomHandler::generatePathways()
{	
	glm::vec3 delta{};
	glm::vec3 step{};
	glm::vec3 curPos{};
	Entity entity = -1;

	// Used when generating borders
	const glm::vec3 OFFSETS[4] =
	{
		{TILE_WIDTH, 0, 0},
		{-TILE_WIDTH, 0, 0},
		{0, 0, TILE_WIDTH},
		{0, 0, -TILE_WIDTH}
	};

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

		// Calcuate the path, place tiles and save startIndex
		size_t startIndex = this->pathIds.size();
		while(true)
		{
			this->pathIds.emplace_back(this->createPathEntity());
			this->scene->getComponent<Transform>(this->pathIds.back()).position = curPos;
			
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

		const size_t endIndex = this->pathIds.size();
		bool canPlace = true;

		// Go through the placed path and generate a border around it
		for (size_t j = startIndex; j < endIndex; j++)
		{
			const glm::vec3& pos = this->scene->getComponent<Transform>(this->pathIds[j]).position;
			for (int k = 0; k < 4; k++)
			{
				const glm::vec3 offsetPos = pos + OFFSETS[k];
				canPlace = true;

				// Don't place a border inside the room
				if (this->verticalConnection[i])
				{
					if (std::abs(offsetPos.z - (p0.z - step.z)) < TILE_WIDTH * 0.1f || 
						std::abs(offsetPos.z - (p1.z + step.z)) < TILE_WIDTH * 0.1f)
					{
						canPlace = false;
					}
				}
				else
				{
					if (std::abs(offsetPos.x - (p0.x - step.x)) < TILE_WIDTH * 0.1f ||
						std::abs(offsetPos.x - (p1.x + step.x)) < TILE_WIDTH * 0.1f)
					{
						canPlace = false;
					}
				}

				// Search through the path to see if offsetPos is on a tile
				for (size_t m = startIndex; m < this->pathIds.size() && canPlace; m++)
				{
					if (m != j)
					{
						glm::vec3 mPosToOffset = offsetPos - this->scene->getComponent<Transform>(this->pathIds[m]).position;
						mPosToOffset.y = 0.f;

						if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
						{
							canPlace = false;
							m = this->pathIds.size();
						}
					}
				}

				if (canPlace)
				{
					entity = createPathBorderEntity(offsetPos);
					this->pathIds.emplace_back(entity);
				}
			}
		}
	}
}

void RoomHandler::roomToWorldSpace(int index)
{	
	Room& room = this->rooms[index];
	const glm::vec3& roomPos = this->roomLayout.getRoom(index).position;

	// Scale doors
	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			Transform& tra = this->scene->getComponent<Transform>(room.doors[i]);
			tra.position *= TILE_WIDTH;
			tra.position += roomPos;
			tra.scale *= TILE_WIDTH;

			this->roomExitPoints[index].positions[i] *= TILE_WIDTH;
			this->roomExitPoints[index].positions[i] += roomPos;
		}
	}

	// Scale all tiles
	for (Entity id : room.tiles)
	{
		Transform& tra = this->scene->getComponent<Transform>(id);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
	for (Entity id : room.borders)
	{
		Transform& tra = this->scene->getComponent<Transform>(id);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
	for (Entity id : room.exitPaths)
	{
		Transform& tra = this->scene->getComponent<Transform>(id);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
}

void RoomHandler::createColliders()
{
	const float halfTile = TILE_WIDTH * 0.5f;
	const float offset = TILE_WIDTH * 0.7f;
	const glm::vec3 offsets[4] = 
	{
		glm::vec3(-offset, halfTile, 0.f),
		glm::vec3(offset, halfTile, 0.f),
		glm::vec3(0.f, halfTile, -offset),
		glm::vec3(0.f, halfTile, offset)
	};

	const glm::vec3 borderColDims(halfTile, TILE_WIDTH * 2.f, halfTile);
	const glm::vec3 doorColDims(halfTile, TILE_WIDTH, TILE_WIDTH * 0.1f);
	const glm::vec3 doorTrigCol(halfTile, halfTile, TILE_WIDTH - offset);

	for (Room& room : this->rooms)
	{
		for (Entity id : room.borders)
		{
			this->scene->setComponent<Collider>(id, Collider::createBox(borderColDims));
		}

		for (int i = 0; i < 4; i++)
		{
			if (room.doors[i] != -1)
			{
				// Not creating a collider for the doors, all doors are open when the game starts

				// Trigger
				room.doorTriggers[i] = this->scene->createEntity();
				
				Transform& doorTra = this->scene->getComponent<Transform>(room.doors[i]);
				Transform& triggerTra = this->scene->getComponent<Transform>(room.doorTriggers[i]);

				triggerTra.position = doorTra.position + offsets[i];
				triggerTra.rotation = doorTra.rotation;
				
				this->scene->setComponent<Collider>(room.doorTriggers[i], Collider::createBox(doorTrigCol, true));

			}
		}
	}

	for (Entity entity : this->pathIds)
	{
		const int meshID = this->scene->getComponent<MeshComponent>(entity).meshID;
		for (int i = 0; i < (int)borderMeshIds.size(); i++)
		{
			if (meshID == (int)borderMeshIds[i])
			{
				this->scene->setComponent<Collider>(entity, Collider::createBox(borderColDims));
				i = (int)borderMeshIds.size();
			}
		}
	}

	float minX = 10000000.f;
	float maxX = -10000000.f;
					  
	float minZ = 10000000.f;
	float maxZ = -10000000.f;

	for (int i = 0; i < this->roomLayout.getNumRooms(); i++)
	{
		const glm::vec3& pos = this->roomLayout.getRoom(i).position;

		if		(pos.x < minX) { minX = pos.x; }
		else if (pos.x > maxX) { maxX = pos.x; }
		
		if		(pos.z < minZ) { minZ = pos.z; }
		else if (pos.z > maxZ) { maxZ = pos.z; }
	}

	const float HalfRoom = (float)this->roomGenerator.getRoomSize() * TILE_WIDTH * 0.5f + TILE_WIDTH * 0.5f;

	const glm::vec3 floorPos((minX + maxX) * 0.5f, -4.f, (minZ + maxZ) * 0.5f);
	const glm::vec3 floorDimensions((maxX - minX) * 0.5f + HalfRoom, 4.f, (maxZ - minZ) * 0.5f + HalfRoom);

	this->floor = this->scene->createEntity();
	this->scene->setComponent<Collider>(this->floor, Collider::createBox(floorDimensions));
	this->scene->getComponent<Transform>(this->floor).position = floorPos;
}


Entity RoomHandler::createTileEntity(int tileIndex, TileUsage usage)
{
	Tile tile{};
	int meshId = -1;
	switch (usage)
	{
	case RoomHandler::Default:
		tile = this->roomGenerator.getTile(tileIndex);
		if (rand() % 3 < 2) {meshId = (int)this->oneXOneMeshIds[0]; }
		else { meshId = (int)this->oneXOneMeshIds[rand() % (NUM_ONE_X_ONE - 1) + 1]; }
		break;
	case RoomHandler::Border:
		tile = this->roomGenerator.getBorder(tileIndex);
		meshId = (int)this->borderMeshIds[rand() % NUM_BORDER];
		break;
	case RoomHandler::Exit:
		tile = this->roomGenerator.getExitTiles(tileIndex);
		meshId = (int)this->oneXOneMeshIds[rand() % NUM_ONE_X_ONE];
		break;

	default:
		break;
	}

	Entity pieceID = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = meshId;

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);
	transform.position = glm::vec3(tile.position.x, 0.f, tile.position.y);
	transform.rotation.y = (rand() % 4) * 90.f;

	return pieceID;
}

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = this->doorMeshID;

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;

	return entity;
}

Entity RoomHandler::createPathEntity()
{
	Entity entity = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(entity);

	int meshId;
	if (rand() % 3 < 2) { meshId = (int)this->oneXOneMeshIds[0]; }
	else { meshId = (int)this->oneXOneMeshIds[rand() % (NUM_ONE_X_ONE - 1) + 1]; }

	this->scene->getComponent<MeshComponent>(entity).meshID = meshId;
	
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE) * TILE_WIDTH;
	transform.rotation.y = (rand() % 4) * 90.f;

	return entity;
}

Entity RoomHandler::createPathBorderEntity(const glm::vec3& position)
{
	Entity entity = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = (int)this->borderMeshIds[rand() % NUM_BORDER];

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.position = position;
	transform.scale *= RoomGenerator::DEFAULT_TILE_SCALE * TILE_WIDTH;
	transform.rotation.y = (rand() % 4) * 90.f;

	return entity;
}

bool RoomHandler::checkRoom(int index, Entity otherEntity)
{
	bool newUnfinshedRoom = false;
	Room& curRoom = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		Entity id = curRoom.doorTriggers[i];

		// Will also skip invalid doors (-1)
		if (otherEntity == id)
		{
			// Entered next room
			if (index == this->nextIndex)
			{
				if (!curRoom.finished)
				{
					newUnfinshedRoom = true;

#ifdef _CONSOLE
					if (!showAllRooms)
					{
						this->deactivateRoom(this->activeIndex);
						this->showPaths(false);
					}
#else
					this->deactivateRoom(this->activeIndex);
					this->showPaths(false);
#endif
					this->closeDoors(index);

					this->nextIndex = -1;
					this->curDoor = -1;
				}
				else
				{
					this->curDoor = i;
					this->nextIndex = curRoom.connectingIndex[i];
				}

				this->activeIndex = index;
			}
	
			// Leaving room
			else if (index == this->activeIndex && this->curDoor != i && curRoom.finished)
			{
#ifdef _CONSOLE
				if (!showAllRooms)
				{
					this->deactivateRoom(this->nextIndex);
					this->activateRoom(curRoom.connectingIndex[i]);
				}
#else
				this->deactivateRoom(this->nextIndex);
				this->activateRoom(curRoom.connectingIndex[i]);
#endif

				this->curDoor = i;
				this->nextIndex = curRoom.connectingIndex[i];
			}

			break;
		}
	}

	return newUnfinshedRoom;
}

void RoomHandler::showPaths(bool show)
{
	if (show)
	{
		for (Entity tile : this->pathIds)
		{
			this->scene->setActive(tile);
		}
	}
	else
	{
		for (Entity tile : this->pathIds)
		{
			this->scene->setInactive(tile);
		}
	}
}


void RoomHandler::reset()
{
	for (Room& room : this->rooms)
	{
		for (const Entity& id : room.tiles)
		{
			this->scene->removeEntity(id);
		}
		for (const Entity& id : room.borders)
		{
			this->scene->removeEntity(id);
		}
		for (const Entity& id : room.exitPaths)
		{
			this->scene->removeEntity(id);
		}
		room.tiles.clear();
		room.borders.clear();
		room.exitPaths.clear();

		for (int i = 0; i < 4; i++)
		{
			if (room.doors[i] != -1)
			{
				this->scene->removeEntity(room.doors[i]);
				room.doors[i] = -1;

				this->scene->removeEntity(room.doorTriggers[i]);
				room.doorTriggers[i] = -1;
			}
		}

		room.finished = false;
	}																				  

	for (int& id : this->pathIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	this->pathIds.clear();
	
	this->scene->removeEntity(this->floor);
	this->floor = -1;

	this->exitPairs.clear();
	this->roomExitPoints.clear();

	this->activeIndex = 0;
	this->nextIndex = -1;
	this->curDoor = -1;
}

void RoomHandler::openDoors(int index)
{
	if (index < 0 || index >= (int)this->rooms.size())
	{
		return;
	}

	Room& room = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			this->scene->setScriptComponent(room.doors[i], "scripts/opendoor.lua");
			this->scene->removeComponent<Collider>(room.doors[i]);
		}
	}
}

void RoomHandler::closeDoors(int index)
{
	if (index < 0 || index >= (int)this->rooms.size())
	{
		return;
	}

	Room& room = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			this->scene->setScriptComponent(room.doors[i], "scripts/closedoor.lua");
			this->scene->setComponent<Collider>(room.doors[i], 
				Collider::createBox(glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH, TILE_WIDTH * 0.1f)));
		}
	}
}


void RoomHandler::activateRoom(int index)
{
	if (index < 0 || index >= (int)this->rooms.size())
	{
		return;
	}

	Room& curRoom = this->rooms[index];
	for (const Entity& id : curRoom.tiles)
	{
		this->scene->setActive(id);
	}
	for (const Entity& id : curRoom.borders)
	{
		this->scene->setActive(id);
	}
	for (const Entity& id : curRoom.exitPaths)
	{
		this->scene->setActive(id);
	}
	
	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			 this->scene->setActive(curRoom.doors[i]);
			 this->scene->setActive(curRoom.doorTriggers[i]);
			 this->scene->getComponent<Transform>(curRoom.doors[i]).position.y = -25;
		}
	}
}

void RoomHandler::deactivateRoom(int index)
{
	if (index < 0 || index >= (int)this->rooms.size())
	{
		return;
	}

	Room& curRoom = this->rooms[index];
	for (const Entity& id : curRoom.tiles)
	{
		this->scene->setInactive(id);
	}
	for (const Entity& id : curRoom.borders)
	{
		this->scene->setInactive(id);
	}
	for (const Entity& id : curRoom.exitPaths)
	{
		this->scene->setInactive(id);
	}
	
	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			 this->scene->setInactive(curRoom.doors[i]);
			 this->scene->setInactive(curRoom.doorTriggers[i]);
		}
	}
}

#ifdef _CONSOLE
void RoomHandler::imgui()
{
	if (ImGui::Begin("Debug"))
	{
		ImGui::PushItemWidth(-100.f);
		ImGui::Text("Rooms");
		ImGui::Text("Num: %zd", this->rooms.size());

		if (ImGui::Checkbox("Show all rooms", &this->showAllRooms))
		{
			if (this->showAllRooms)
			{ 
				for (int i = 0; i < (int)this->rooms.size(); i++)
				{
					this->activateRoom(i);	
				}
				this->showPaths(true);
			}
			else 
			{
				for (int i = 0; i < (int)this->rooms.size(); i++)
				{
					if (i == this->activeIndex || i == this->nextIndex)
					{
						this->activateRoom(i);
					}
					else
					{
						this->deactivateRoom(i);
					}

				}
			}
		}

		if (ImGui::Button("Reload")) 
		{
			this->generate();
		} 

		ImGui::Text("A: %d, N: %d, D: %d", this->activeIndex, this->nextIndex, this->curDoor);

		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();
}

#endif // _CONSOLE

