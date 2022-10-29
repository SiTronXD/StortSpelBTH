#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::TILE_WIDTH = 24.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 3;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), hasDoor{},
	activeIndex(0), nextIndex(-1), floor(-1),
	openDoorMeshID(0), closedDoorMeshID(0), gridSize(0)
{
	srand((unsigned)time(0));
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->gridSize = roomSize;

	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.setRoomDistance(TILE_WIDTH * roomSize + TILE_WIDTH * TILES_BETWEEN_ROOMS);	

	// Border pieces not counted in tileTypes
	for (int i = 0; i < tileTypes + 1; i++)
	{
		// Tile types in RoomGenerator ranges from 0-tileTypes
		uint32_t id = resourceMan->addMesh("assets/models/room_piece_" + std::to_string(i) + ".obj");
		this->tileMeshIds[Tile::Type(i)] = id;
	}

	this->openDoorMeshID = resourceMan->addMesh("assets/models/doorOpen.obj");
	this->closedDoorMeshID = resourceMan->addMesh("assets/models/doorClosed.obj");
}

void RoomHandler::roomCompleted()
{
	this->roomFinished = true;
	this->rooms[this->activeIndex].finished = true;

	this->openDoors(this->activeIndex);
	this->showPaths(true);
}

bool RoomHandler::newRoom(const glm::vec3& playerPos, Entity entity)
{
	// Check if passed through door

#ifdef _CONSOLE
	if (!this->showAllRooms)
	{
		if (this->checkRoom(this->activeIndex, playerPos, entity))
		{
			return true;
		}

		if (this->nextIndex != -1)
		{ 
			if (this->checkRoom(this->nextIndex, playerPos, entity))
			{
				return true;
			}
		}
	}
#else
	if (this->checkRoom(this->activeIndex, playerPos, entity))
	{
		return true;
	}

	if (this->nextIndex != -1)
	{ 
		if (this->checkRoom(this->nextIndex, playerPos, entity))
		{
			return true;
		}
	}
#endif // _CONSOLE

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
		// Reset the generator
		this->roomGenerator.reset();

		const RoomLayout::RoomData& roomRef = this->roomLayout.getRoom(i);
		const glm::vec3& roomPos = roomRef.position;

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
			this->rooms[i].tiles.emplace_back(this->createTileEntity(j, roomPos));
		}

		this->createDoors(i);
		this->roomGenerator.generateBorders(this->hasDoor);

		// Save room exit points
		this->setExitPoints(i);

		this->rooms[i].borders.reserve(size_t(this->roomGenerator.getNrBorders()));
		for (int j = 0; j < this->roomGenerator.getNrBorders(); j++) 
		{
			this->rooms[i].borders.emplace_back(this->createBorderEntity(j, roomPos));
		}

		// Save exit paths
		this->rooms[i].exitPaths.reserve(size_t(this->roomGenerator.getNrExitTiles()));
		for (int j = 0; j < this->roomGenerator.getNrExitTiles(); j++) 
		{
			this->rooms[i].exitPaths.emplace_back(this->createExitTileEntity(j, roomPos));
		}

	
		// Scale the room, RoomGenerator is dependant on a tile being 1x1
		this->scaleRoom(i, roomPos);
	}

	// Find exit pairs and create pathways
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

				pair.first = this->roomExitPoints[curCon.y].worldPositions[2];
				pair.first.z += TILE_WIDTH;
				
				pair.second = this->roomExitPoints[curCon.x].worldPositions[3];
				pair.second.z -= TILE_WIDTH;
			}
			else
			{
				// Horizontal connection
				// curCon.x = right room
				// curCon.y = left room

				this->verticalConnection[i] = false;

				pair.first = this->roomExitPoints[curCon.y].worldPositions[0];
				pair.first.x += TILE_WIDTH;

				pair.second = this->roomExitPoints[curCon.x].worldPositions[1];
				pair.second.x -= TILE_WIDTH;
			}
		}

		this->generatePathways();
	}
	
	this->createColliders();
	this->roomLayout.clear();

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

		this->closeDoors(this->activeIndex);
		this->showPaths(false);
	}
#else
	for (int i = 0; i < (int)this->rooms.size(); i++)
	{
		if (i != this->activeIndex)
		{
			this->deactivateRoom(i);
		}
	}

	this->closeDoors(this->activeIndex);
	this->showPaths(false);
#endif // _CONSOLE

}

const std::vector<Entity>& RoomHandler::getFreeTiles()
{
	return this->rooms[this->activeIndex].tiles;
}

void RoomHandler::createDoors(int roomIndex)
{
	const RoomLayout::RoomData& curRoom = this->roomLayout.getRoom(roomIndex);
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

void RoomHandler::setExitPoints(int roomIndex)
{
	const glm::vec2* exitPositions = this->roomGenerator.getExitTilesPos();

	this->roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (this->rooms[roomIndex].doors[i] != -1)
		{ 
			this->roomExitPoints.back().worldPositions[i] = glm::vec3(exitPositions[i].x, 0.f, exitPositions[i].y);
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
			this->scene->getComponent<Transform>(this->pathIds.back()).position = snapToGrid(curPos);
			
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
						const glm::vec3 mPosToOffset = offsetPos - this->scene->getComponent<Transform>(this->pathIds[m]).position;
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

void RoomHandler::scaleRoom(int index, const glm::vec3& roomPos)
{	
	Room& room = this->rooms[index];

	// Scale doors
	for (int i = 0; i < 4; i++)
	{
		if (room.doors[i] != -1)
		{
			Transform& tra = this->scene->getComponent<Transform>(room.doors[i]);
			tra.position *= TILE_WIDTH;
			tra.position += roomPos;
			tra.scale *= TILE_WIDTH;

			this->roomExitPoints[index].worldPositions[i] *= TILE_WIDTH;
			this->roomExitPoints[index].worldPositions[i] += roomPos;
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

	const glm::vec3 borderColDims(halfTile, TILE_WIDTH, halfTile);
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
		if (this->scene->getComponent<MeshComponent>(entity).meshID == this->tileMeshIds[Tile::Border])
		{
			this->scene->setComponent<Collider>(entity, Collider::createBox(borderColDims));
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

	const float HalfRoom = (float)this->gridSize * TILE_WIDTH * 0.5f + TILE_WIDTH * 0.5f;

	const glm::vec3 floorPos((minX + maxX) * 0.5f, -4.f, (minZ + maxZ) * 0.5f);
	const glm::vec3 floorDimensions((maxX - minX) * 0.5f + HalfRoom, 4.f, (maxZ - minZ) * 0.5f + HalfRoom);

	this->floor = this->scene->createEntity();
	this->scene->setComponent<Collider>(this->floor, Collider::createBox(floorDimensions));
	this->scene->getComponent<Transform>(this->floor).position = floorPos;
}

Entity RoomHandler::createTileEntity(int tileIndex, const glm::vec3& roomPos)
{
	Entity pieceID = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[this->roomGenerator.getTile(tileIndex).type];

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);

	transform.position = glm::vec3(
		this->roomGenerator.getTile(tileIndex).position.x,
		0.f,
		this->roomGenerator.getTile(tileIndex).position.y);

	return pieceID;
}

Entity RoomHandler::createBorderEntity(int tileIndex, const glm::vec3& roomPos)
{
	Entity pieceID = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[Tile::Border];

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);


	transform.position = glm::vec3(
		this->roomGenerator.getBorder(tileIndex).position.x,
		0.f,
		this->roomGenerator.getBorder(tileIndex).position.y);

	return pieceID;
}

Entity RoomHandler::createExitTileEntity(int tileIndex, const glm::vec3& roomPos)
{
	Entity pieceID = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[Tile::OneXOne];

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);

	transform.position = glm::vec3(
		this->roomGenerator.getExitTiles(tileIndex).position.x,
		0.f,
		this->roomGenerator.getExitTiles(tileIndex).position.y);

	return pieceID;
}

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = this->openDoorMeshID;

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;

	return entity;
}

Entity RoomHandler::createPathEntity()
{
	Entity id = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(id);
	this->scene->getComponent<MeshComponent>(id).meshID = this->tileMeshIds[Tile::OneXOne];
	
	this->scene->getComponent<Transform>(id).scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE) * TILE_WIDTH;

	return id;
}

Entity RoomHandler::createPathBorderEntity(const glm::vec3& position)
{
	Entity entity = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = this->tileMeshIds[Tile::Border];
	Transform& tra = this->scene->getComponent<Transform>(entity);
	tra.position = position;
	tra.scale *= RoomGenerator::DEFAULT_TILE_SCALE * TILE_WIDTH;

	return entity;
}

bool RoomHandler::checkRoom(int index, const glm::vec3& playerPos, Entity entity)
{
	this->insideDoor = false;
	bool result = false;

	Room& curRoom = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		Entity id = curRoom.doorTriggers[i];

		// Will also skip invalid doors (-1)
		if (entity == id)
		{
			const Collider& col = this->scene->getComponent<Collider>(id);

			// Entered new room
			if (index == this->nextIndex && !curRoom.finished)
			{
				// deactivate the old room
				this->deactivateRoom(this->activeIndex);

				this->activeIndex = index;
				this->nextIndex = -1;
				this->roomFinished = false;
				result = true;

				// Close doors
				this->closeDoors(this->activeIndex);
				this->showPaths(false);
			}
#if 0
			

			else if (index == this->activeIndex && this->curDoor != i)
			{
				this->deactivateRoom(this->nextIndex);

				this->curDoor = i;
				this->nextIndex = curRoom.connectingIndex[i];

				this->activateRoom(this->nextIndex);
			}

#elif 1

			// Left finished room
			else if (this->curDoor == -1 && this->roomFinished)
			{
				this->activeIndex = index;
				this->nextIndex = curRoom.connectingIndex[i];
				this->curDoor = i;
				
				this->activateRoom(this->nextIndex);
				printf("Bai?\n");

				//flipDoors(true);
			}

			else if (index == this->activeIndex && this->curDoor != i && this->roomFinished)
			{
				this->deactivateRoom(this->nextIndex);

				this->curDoor = i;
				this->nextIndex = curRoom.connectingIndex[i];

				this->activateRoom(this->nextIndex);
				//flipDoors(true);

				printf("Passing through...\n");
			}
#endif

			this->insideDoor = true;
		}
	}
	if (index == this->activeIndex && !this->insideDoor)
	{ 
		this->curDoor = -1;
	}

	return result;
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
			this->scene->getComponent<MeshComponent>(room.doors[i]).meshID = openDoorMeshID;
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
			this->scene->getComponent<MeshComponent>(room.doors[i]).meshID = closedDoorMeshID;
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

glm::vec3 RoomHandler::snapToGrid(const glm::vec3& pos)
{
	return glm::vec3(
		std::floor(pos.x / TILE_WIDTH) * TILE_WIDTH, 
		0.f, 
		std::floor(pos.z / TILE_WIDTH) * TILE_WIDTH);
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

