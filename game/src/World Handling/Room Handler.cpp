#include "Room Handler.h"
#include "vengine/application/Scene.hpp"
//#include "vengine/physics/PhysicsEngine.h"

const float RoomHandler::TILE_WIDTH = 25.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 5;
const uint32_t RoomHandler::NUM_BORDER = 1;
const uint32_t RoomHandler::NUM_ONE_X_ONE = 4;
const uint32_t RoomHandler::NUM_ONE_X_TWO = 1;
const uint32_t RoomHandler::NUM_TWO_X_TWO = 1;

RoomHandler::RoomHandler()
	:scene(nullptr), hasDoor{false, false, false, false},
	activeIndex(0), nextIndex(-1), floor(-1),
	doorMeshID(0), tileFlorMeshId(0)
{
}

RoomHandler::~RoomHandler()
{
}

/*
 No path borders?
 Wrong pos for 2x2 ? 

*/

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

	this->oneXTwoMeshIds.resize(NUM_ONE_X_TWO);
	for (uint32_t i = 0; i < NUM_ONE_X_TWO; i++)
	{
		this->oneXTwoMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
	}

	this->twoXTwoMeshIds.resize(NUM_TWO_X_TWO);
	for (uint32_t i = 0; i < NUM_TWO_X_TWO; i++)
	{
		this->twoXTwoMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
	}

	this->doorMeshID = resourceMan->addMesh("assets/models/door.obj");
	this->tileFlorMeshId = resourceMan->addMesh("assets/models/Tiles/Floor.obj");

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

		this->rooms[i].mainTiles.reserve(size_t(this->roomGenerator.getNrTiles()));
		for (int j = 0; j < this->roomGenerator.getNrTiles(); j++) 
		{
			this->rooms[i].mainTiles.emplace_back(this->createTileEntity(j, TileUsage::Default));
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
		for (int i = 0; i < numRooms; i++)
		{
			this->activateRoom(i);
		}
		this->showPaths(true);
	}
	else
	{
		for (int i = 0; i < numRooms; i++)
		{
			if (i != this->activeIndex) 
			{ 
				this->deactivateRoom(i); 
			}
		}
		this->showPaths(true);
	}
#else
	for (int i = 0; i < numRooms; i++)
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

void RoomHandler::generate2()
{
	this->reset();

	this->roomLayout.generate();
	const int numTotRooms = this->roomLayout.getNumRooms();
	const int numMainRooms = this->roomLayout.getNumMainRooms();
	this->rooms.resize(numTotRooms);

	this->roomGen.setDesc(roomGenDesc);
	this->roomExitPoints.resize(numTotRooms);
	static int rCount = 0;
	rCount++;
	for (int i = 0; i < numTotRooms; i++)
	{
		const RoomData& roomData = this->roomLayout.getRoom(i);
		Room& curRoom = this->rooms[i];

		//curRoom.position = roomData.position;
		curRoom.type = roomData.type;

		// Save connecting indices
		curRoom.connectingIndex[0] = roomData.left;
		curRoom.connectingIndex[1] = roomData.right;
		curRoom.connectingIndex[2] = roomData.up;
		curRoom.connectingIndex[3] = roomData.down;

		hasDoor[0] = roomData.left != -1;
		hasDoor[1] = roomData.right != -1;
		hasDoor[2] = roomData.up != -1;
		hasDoor[3] = roomData.down != -1;

		this->roomGen.generate(hasDoor);
		const glm::ivec2* exits = this->roomGen.getExits();

		curRoom.extents[0] = std::abs((float)exits[0].x) * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[1] = std::abs((float)exits[1].x) * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[2] = std::abs((float)exits[2].y) * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[3] = std::abs((float)exits[3].y) * TILE_WIDTH - TILE_WIDTH * 0.5f;

		createDoors(i);

		for (int j = 0; j < 4; j++)
		{
			if (curRoom.connectingIndex[j] != -1)
			{
				this->roomExitPoints[i].positions[j] = glm::vec3(exits[j].x, 0.f, exits[j].y) * TILE_WIDTH;
			}
		}

		const uint32_t numSmall = this->roomGen.getNumMainTiles();
		curRoom.mainTiles.resize(size_t(numSmall));
		for (uint32_t j = 0; j < numSmall; j++)
		{
			curRoom.mainTiles[j] = createFloorEntity(this->roomGen.getMainTile(j).position);
		}

		const uint32_t numBig = this->roomGen.getNumBigTiles();
		curRoom.objects.resize(size_t(numBig));

		for (uint32_t j = 0; j < numBig; j++)
		{
			const Tile2& tile = roomGen.getBigTile(j);

			Entity entity = this->scene->createEntity();
			Transform& transform = this->scene->getComponent<Transform>(entity);

			this->scene->setComponent<MeshComponent>(entity);
			if (tile.type == Tile2::TwoXTwo)
				this->scene->getComponent<MeshComponent>(entity).meshID = twoXTwoMeshIds[0];
			else if (tile.type == Tile2::TwoXOne)
				this->scene->getComponent<MeshComponent>(entity).meshID = oneXTwoMeshIds[0];
			else if (tile.type == Tile2::OneXTwo)
			{
				this->scene->getComponent<MeshComponent>(entity).meshID = oneXTwoMeshIds[0];
				transform.rotation.y = 90.f;
			}

			transform.position = glm::vec3(tile.position.x, 0.f, tile.position.y);
			transform.position *= TILE_WIDTH;
			if (tile.type == Tile2::TwoXOne || tile.type == Tile2::OneXTwo)
				transform.scale.y = 10.f;

			curRoom.objects[j] = entity;
		}

		curRoom.exitPaths.resize(size_t(this->roomGen.getNumExitTiles()));
		for (uint32_t j = 0; j < this->roomGen.getNumExitTiles(); j++)
		{
			curRoom.exitPaths[j] = createFloorEntity(this->roomGen.getExitTile(j).position);
		}

		curRoom.borders.resize(size_t(this->roomGen.getNumBorders()));
		for (uint32_t j = 0; j < this->roomGen.getNumBorders(); j++)
		{
			curRoom.borders[j] = createBorderEntity(this->roomGen.getBorder(j).position, true);
		}

		this->roomGen.clear();
	}

	for (int i = 0; i < numMainRooms; i++)
	{
		if (rooms[i].connectingIndex[3] != -1)
		{
			Room& other = rooms[rooms[i].connectingIndex[3]];
			float offset = other.position.z + other.extents[2] + rooms[i].extents[3]
				+ TILES_BETWEEN_ROOMS * TILE_WIDTH;

			moveRoom(i, glm::vec3(0.f, 0.f, offset));
		}

		placeBranch(i, rooms[i].connectingIndex[0], rooms[i].connectingIndex[1]);
	}

	this->setConnections();
	this->generatePathways();
	this->createColliders();

	this->roomLayout.clear();
}

void RoomHandler::placeBranch(int index, int left, int right)
{
	Room& curRoom = rooms[index];
	glm::vec3 offset = curRoom.position;
	
	if (left != -1)
	{
		Room& leftRoom = rooms[left];
		offset.x += curRoom.extents[0] + leftRoom.extents[1] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		moveRoom(left, offset);

		if (leftRoom.connectingIndex[0] != -1)
		{
			placeBranch(left, leftRoom.connectingIndex[0], -1);
		}
	}
	
	offset = curRoom.position;
	if (right != -1)
	{
		Room& rightRoom = rooms[right];
		offset.x -= curRoom.extents[1] + rightRoom.extents[0] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		moveRoom(right, offset);

		if (rightRoom.connectingIndex[1] != -1)
		{
			placeBranch(right, -1, rightRoom.connectingIndex[1]);
		}
	}
}

void RoomHandler::moveRoom(int roomIndex, glm::vec3 offset)
{
	Room& curRoom = rooms[roomIndex];

	curRoom.position += offset;
			
	for (int i = 0; i < 4; i++)
	{
		roomExitPoints[roomIndex].positions[i] += offset;
		if (curRoom.doors[i] != -1)
		{
			Transform& tra = this->scene->getComponent<Transform>(curRoom.doors[i]);
			tra.position += offset;
		}
	}

	for (Entity entity : curRoom.mainTiles)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position += offset;
	}
	for (Entity entity : curRoom.objects)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position += offset;
	}
	for (Entity entity : curRoom.borders)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position += offset;
	}
	for (Entity entity : curRoom.exitPaths)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position += offset;
	}
}

Entity RoomHandler::createFloorEntity(const glm::vec2& pos)
{
	Entity entity = scene->createEntity();
	scene->setComponent<MeshComponent>(entity, tileFlorMeshId);
	Transform& transform = scene->getComponent<Transform>(entity);
	transform.position.x = pos.x * TILE_WIDTH;
	transform.position.z = pos.y * TILE_WIDTH;
	transform.scale *= TILE_WIDTH;

	return entity;
}

const std::vector<Entity>& RoomHandler::getFreeTiles()
{
	return this->rooms[this->activeIndex].mainTiles;
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

Entity RoomHandler::getFloor() const
{
	return this->floor;
}

void RoomHandler::createDoors(int roomIndex)
{
	Room& curRoom = this->rooms[roomIndex];

	const glm::ivec2* doorTilePos = this->roomGen.getExits();
	const float rots[4] = {-90.f, 90.f, 180.f, 0.f};

	const float OFFSET = 2.5f;
	const glm::vec2 OFFSETS[4] = 
	{
		glm::vec2(-OFFSET, 0.f),
		glm::vec2(OFFSET, 0.f),
		glm::vec2(0.f, -OFFSET),
		glm::vec2(0.f, OFFSET)
	};

	for (int i = 0; i < 4; i++)
	{
		if (this->hasDoor[i])
		{
			curRoom.doors[i] = this->createDoorEntity(rots[i]);

			Transform& tra = this->scene->getComponent<Transform>(curRoom.doors[i]);
			tra.position.x = ((float)doorTilePos[i].x + OFFSETS[i].x) * TILE_WIDTH;
			tra.position.z = ((float)doorTilePos[i].y + OFFSETS[i].y) * TILE_WIDTH;
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
			pair.second = this->roomExitPoints[curCon.x].positions[3];
		}
		else
		{
			// Horizontal connection
			// curCon.x = right room
			// curCon.y = left room

			this->verticalConnection[i] = false;

			pair.first = this->roomExitPoints[curCon.y].positions[0];
			pair.second = this->roomExitPoints[curCon.x].positions[1];
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
	const int ThiccNessMonster = 4;
	const glm::vec3 OFFSETS[] =
	{
		{TILE_WIDTH, 0, 0},
		{-TILE_WIDTH, 0, 0},
		{0, 0, TILE_WIDTH},
		{0, 0, -TILE_WIDTH},
	};
	const int NUM_OFFSETS = sizeof(OFFSETS) / sizeof(glm::vec3);


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

		glm::vec3 upperP = p0.z > p1.z ? p0 : p1;
		glm::vec3 lowerP = upperP == p0 ? p1 : p0;
		
		glm::vec3 rightP = p0.x > p1.x ? p0 : p1;
		glm::vec3 leftP = rightP == p0 ? p1 : p0;

		// Go through the placed path and generate a border around it
		for (size_t j = startIndex; j < endIndex; j++)
		{
			const glm::vec3& pos = this->scene->getComponent<Transform>(this->pathIds[j]).position;
			for (int k = 0; k < NUM_OFFSETS; k++)
			{
				for (int l = 1; l < ThiccNessMonster + 1; l++) // ThiccNess off borders around paths
				{
					const glm::vec3 offsetPos = pos + OFFSETS[k] * (float)l;
					canPlace = true;

					// Don't place a border inside the room
					if (this->verticalConnection[i])
					{
						canPlace = !(offsetPos.z > upperP.z || offsetPos.z < lowerP.z);
					}
					else
					{
						canPlace = !(offsetPos.x > rightP.x || offsetPos.x < leftP.x);
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
						entity = createBorderEntity({offsetPos.x, offsetPos.z}, false);
						this->pathIds.emplace_back(entity);
					}
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
	for (Entity entity : room.mainTiles)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
	for (Entity entity : room.borders)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
	for (Entity entity : room.exitPaths)
	{
		Transform& tra = this->scene->getComponent<Transform>(entity);
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
		for (Entity entity : room.borders)
		{
			this->scene->setComponent<Collider>(entity, Collider::createBox(borderColDims));
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
				
				this->scene->setComponent<Collider>(
                    room.doorTriggers[i],
                    Collider::createBox(doorTrigCol, glm::vec3(0, 0, 0), true)
                );

			}
		}
	}

	// TODO: SEPERATE PATH BORDERS FROM PATHIDS
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
	float extents[4]{};

	for (Room& room : this->rooms)
	{
		if (room.position.x < minX) { minX = room.position.x; extents[0] = room.extents[0]; }
		if (room.position.x > maxX) { maxX = room.position.x; extents[1] = room.extents[1]; }
		if (room.position.z < minZ) { minZ = room.position.z; extents[2] = room.extents[2]; }
		if (room.position.z > maxZ) { maxZ = room.position.z; extents[3] = room.extents[3]; }
	}

	const float HalfRoom = (float)this->roomGenerator.getRoomSize() * TILE_WIDTH * 0.5f + TILE_WIDTH * 0.5f;

	const glm::vec3 floorPos((minX + maxX) * 0.5f, -4.f, (minZ + maxZ) * 0.5f);
	const glm::vec3 floorDimensions((maxX - minX) * 0.5f + extents[0] + extents[1], 4.f, (maxZ - minZ) * 0.5f +  + extents[2] + extents[3]);

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

	int meshId = oneXOneMeshIds[0];
	//if (rand() % 3 < 2) { meshId = (int)this->oneXOneMeshIds[0]; }
	//else { meshId = (int)this->oneXOneMeshIds[rand() % (NUM_ONE_X_ONE - 1) + 1]; }

	this->scene->getComponent<MeshComponent>(entity).meshID = meshId;
	
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE) * TILE_WIDTH;
	transform.rotation.y = (rand() % 4) * 90.f;

	return entity;
}

Entity RoomHandler::createBorderEntity(const glm::vec2& position, bool scalePos)
{
	Entity entity = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = (int)this->borderMeshIds[rand() % NUM_BORDER];

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.position.x = position.x * (scalePos ? TILE_WIDTH : 1.f);
	transform.position.z = position.y * (scalePos ? TILE_WIDTH : 1.f);
	return entity;
}

bool RoomHandler::checkRoom(int index, Entity otherEntity)
{
	bool newUnfinshedRoom = false;
	Room& curRoom = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		Entity entity = curRoom.doorTriggers[i];

		// Will also skip invalid doors (-1)
		if (otherEntity == entity)
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
		for (const Entity& entity : room.mainTiles)
		{
			this->scene->removeEntity(entity);
		}
		for (const Entity& entity : room.objects)
		{
			this->scene->removeEntity(entity);
		}
		for (const Entity& entity : room.borders)
		{
			this->scene->removeEntity(entity);
		}
		for (const Entity& entity : room.exitPaths)
		{
			this->scene->removeEntity(entity);
		}
		room.mainTiles.clear();
		room.objects.clear();
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

			room.connectingIndex[i] = -1;
			room.extents[i] = 0.f;
		}
		room.position = glm::vec3(0.f);
		room.finished = false;
	}																				  

	for (const Entity& entity : this->pathIds)
	{
		this->scene->removeEntity(entity);
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
	for (const Entity& entity : curRoom.mainTiles)
	{
		this->scene->setActive(entity);
	}
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setActive(entity);
	}
	for (const Entity& entity : curRoom.borders)
	{
		this->scene->setActive(entity);
	}
	for (const Entity& entity : curRoom.exitPaths)
	{
		this->scene->setActive(entity);
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
	for (const Entity& entity : curRoom.mainTiles)
	{
		this->scene->setInactive(entity);
	}
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setInactive(entity);
	}
	for (const Entity& entity : curRoom.borders)
	{
		this->scene->setInactive(entity);
	}
	for (const Entity& entity : curRoom.exitPaths)
	{
		this->scene->setInactive(entity);
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
void RoomHandler::imgui(PhysicsEngine* physicsEngine)
{
#if 0
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
#else
	if (ImGui::Begin("Rooms"))
	{
		ImGui::PushItemWidth(-100.f);

		int borderSize = roomGenDesc.borderSize;
		int radius = roomGenDesc.radius;
		int numBranches = roomGenDesc.numBranches;
		int branchDepth = roomGenDesc.branchDepth;
		int angle = roomGenDesc.maxAngle;
		int branchDist = roomGenDesc.branchDist;

		ImGui::InputInt("border", &borderSize, 1, 1);
		ImGui::InputInt("radius", &radius, 1, 1);
		ImGui::InputInt("num branch", &numBranches, 1, 1);
		ImGui::InputInt("branch depth", &branchDepth, 1, 1);
		ImGui::InputInt("branch dist", &branchDist, 1, 1);
		ImGui::InputInt("angle", &angle, 1, 10);

		if (radius < 1)			radius = 1;
		if (numBranches < 1)	numBranches = 1;
		if (branchDepth < 1)	branchDepth = 1;
		if (angle < 0)			angle = 0;
		if (branchDist < 1)		branchDist = 1;

		roomGenDesc.borderSize = borderSize;
		roomGenDesc.radius = radius;
		roomGenDesc.numBranches = numBranches;
		roomGenDesc.branchDepth = branchDepth;
		roomGenDesc.maxAngle = angle;
		roomGenDesc.branchDist = branchDist;

		if (ImGui::Button("Reload"))
		{
			generate2();
		}

		if (physicsEngine)
		{
			static bool debugShapes = false;
			ImGui::Checkbox("Debug shapes", &debugShapes);
			physicsEngine->renderDebugShapes(debugShapes);
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif
}

#endif // _CONSOLE

