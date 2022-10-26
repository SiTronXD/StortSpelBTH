#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::TILE_WIDTH = 25.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 3;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), hasDoor{},
	activeIndex(0), nextIndex(-1),
	openDoorMeshID(0), closedDoorMeshID(0)
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

void RoomHandler::update(const glm::vec3& playerPos)
{
	// Check if passed through door

#ifdef _DEBUG
	
	if (ImGui::Begin("Debug"))
	{
		ImGui::PushItemWidth(-100.f);
		ImGui::Text("Rooms");
		ImGui::Text("Num: %zd", this->rooms.size());

		if (ImGui::Checkbox("Show all rooms", &this->showAllRooms))
		{
			if (this->showAllRooms) { this->activateAll(); }
			else { this->setActiveRooms(); }
		}

		if (ImGui::Button("Reload")) 
		{
			this->generate();
		} 

		ImGui::Text("A: %d, N: %d, D: %d", this->activeIndex, this->nextIndex, this->curDoor, (int)this->insideDoor);
		ImGui::Text("Pos: (%d, %d, %d)", (int)playerPos.x, (int)playerPos.y, (int)playerPos.z);

		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();

#endif


	if (Input::isKeyPressed(Keys::E)) 
	{
		this->roomFinished = true;
		this->flipDoors(true);
		this->rooms[this->activeIndex].finished = true;
	}

#ifdef _DEBUG
	if (!this->showAllRooms)
	{
		this->checkRoom(this->activeIndex, playerPos);
		if (this->nextIndex != -1) { this->checkRoom(this->nextIndex, playerPos); };
	}
#else
	this->checkRoom(this->activeIndex, this->playerPos);
	if (this->nextIndex != -1) { this->checkRoom(this->nextIndex, this->playerPos); }
#endif // _DEBUG
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

		// Generate tiles, doors, borders and connection point
		this->roomGenerator.generateRoom();
		this->createDoors(i);
		this->roomGenerator.generateBorders(this->hasDoor);
		this->setExitPoints(i);

		// Create main tiles
		const int NUM_TILES = this->roomGenerator.getNrTiles();
		this->rooms[i].tileIds.resize(NUM_TILES);
		for (int j = 0; j < NUM_TILES; j++) 
		{
			this->rooms[i].tileIds[j] = this->createTileEntity(j, roomPos);
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
	
	this->roomLayout.clear();

#ifdef _DEBUG
	if (this->showAllRooms) { this->activateAll(); }
	else { this->setActiveRooms(); }
#else
	this->setActiveRooms();
#endif // _DEBUG

}

void RoomHandler::createDoors(int roomIndex)
{
	const RoomLayout::RoomData& curRoom = this->roomLayout.getRoom(roomIndex);
	const glm::vec3& roomPos = curRoom.position;

	Room& curRoomIds = this->rooms[roomIndex];

	if (curRoom.left  != -1) { curRoomIds.doorIds[0] = this->createDoorEntity(-90.f); }
	if (curRoom.right != -1) { curRoomIds.doorIds[1] = this->createDoorEntity(90.f);  }
	if (curRoom.up	  != -1) { curRoomIds.doorIds[2] = this->createDoorEntity(180.f); }
	if (curRoom.down  != -1) { curRoomIds.doorIds[3] = this->createDoorEntity(0.f);	  }

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

		if (curRoomIds.doorIds[i] != -1)
		{
			this->hasDoor[i] = true;
			Transform& tra = this->scene->getComponent<Transform>(curRoomIds.doorIds[i]);

			// Offset by 1.5 (1.5 tiles before room scaling)
			tra.position.x = doorTilePos[i].x + OFFSETS[i].x;
			tra.position.z = doorTilePos[i].y + OFFSETS[i].y;
			tra.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);

			this->scene->setComponent<Box2D>(curRoomIds.doorIds[i]);
		}
	}
}

void RoomHandler::setExitPoints(int roomIndex)
{
	const glm::vec2* exitPositions = this->roomGenerator.getExitTilesPos();

	this->roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (this->rooms[roomIndex].doorIds[i] != -1)
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
		if (room.doorIds[i] != -1)
		{
			Transform& tra = this->scene->getComponent<Transform>(room.doorIds[i]);
			tra.position *= TILE_WIDTH;
			tra.position += roomPos;
			tra.scale *= TILE_WIDTH;

			Box2D& trigger = this->scene->getComponent<Box2D>(room.doorIds[i]);
			trigger.extents.x = TILE_WIDTH * 0.5f;
			trigger.extents.y = TILE_WIDTH * 0.5f;

			this->roomExitPoints[index].worldPositions[i] *= TILE_WIDTH;
			this->roomExitPoints[index].worldPositions[i] += roomPos;
		}
	}

	// Scale all tiles
	for (Entity id : room.tileIds)
	{
		Transform& tra = this->scene->getComponent<Transform>(id);
		tra.position *= TILE_WIDTH;
		tra.position += roomPos;

		tra.scale *= TILE_WIDTH;
	}
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

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = this->closedDoorMeshID;

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

void RoomHandler::checkRoom(int index, const glm::vec3& playerPos)
{
	this->insideDoor = false;

	for (int j = 0; j < 4; j++)
	{
		Entity id = this->rooms[index].doorIds[j];
		if (id == -1)
		{
			continue;
		}

		Transform& tra = this->scene->getComponent<Transform>(id);
		Box2D& box = this->scene->getComponent<Box2D>(id);

		if (box.colliding(tra.position, playerPos))
		{
			if (index == this->nextIndex && !this->rooms[index].finished)
			{
				this->activeIndex = index;
				this->nextIndex = -1;
				this->roomFinished = false;

				setActiveRooms();
				flipDoors(false);
			}

			else if (this->curDoor == -1 && this->roomFinished)
			{
				this->activeIndex = index;
				this->nextIndex = this->rooms[index].connectingIndex[j];
				this->curDoor = j;
				
				setActiveRooms();
				flipDoors(true);
			}
			this->insideDoor = true;
		}
	}
	if (index == this->activeIndex && !this->insideDoor)
	{ 
		this->curDoor = -1;
	}
}

void RoomHandler::reset()
{
	for (Room& room : this->rooms)
	{
		for (int& id : room.tileIds)
		{
			this->scene->removeEntity(id);
		}

		if (room.doorIds[0] != -1) { this->scene->removeEntity(room.doorIds[0]); room.doorIds[0] = -1; }
		if (room.doorIds[1] != -1) { this->scene->removeEntity(room.doorIds[1]); room.doorIds[1] = -1; }
		if (room.doorIds[2] != -1) { this->scene->removeEntity(room.doorIds[2]); room.doorIds[2] = -1; }
		if (room.doorIds[3] != -1) { this->scene->removeEntity(room.doorIds[3]); room.doorIds[3] = -1; }

		room.finished = false;
	}																				  

	for (int& id : this->pathIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	this->pathIds.clear();

	this->exitPairs.clear();
	this->roomExitPoints.clear();

	this->activeIndex = 0;
	this->nextIndex = -1;
}

void RoomHandler::setActiveRooms()
{
	const int num = (int)this->rooms.size();
	for (int i = 0; i < num; i++)
	{
		if (i == this->activeIndex || i == this->nextIndex)
		{
			for (Entity id : this->rooms[i].tileIds)
			{
				this->scene->setActive(id);
			}

			if (this->rooms[i].doorIds[0] != -1) { this->scene->setActive(this->rooms[i].doorIds[0]); }
			if (this->rooms[i].doorIds[1] != -1) { this->scene->setActive(this->rooms[i].doorIds[1]); }
			if (this->rooms[i].doorIds[2] != -1) { this->scene->setActive(this->rooms[i].doorIds[2]); }
			if (this->rooms[i].doorIds[3] != -1) { this->scene->setActive(this->rooms[i].doorIds[3]); }
		}
		else
		{
			for (Entity id : rooms[i].tileIds)
			{
				this->scene->setInactive(id);
			}

			if (this->rooms[i].doorIds[0] != -1) { this->scene->setInactive(this->rooms[i].doorIds[0]); }
			if (this->rooms[i].doorIds[1] != -1) { this->scene->setInactive(this->rooms[i].doorIds[1]); }
			if (this->rooms[i].doorIds[2] != -1) { this->scene->setInactive(this->rooms[i].doorIds[2]); }
			if (this->rooms[i].doorIds[3] != -1) { this->scene->setInactive(this->rooms[i].doorIds[3]); }
		}
	}

	if (this->nextIndex == -1)
	{
		for (const Entity& entity : this->pathIds)
		{
			this->scene->setInactive(entity);
		}
	}
	else
	{
		for (const Entity& entity : this->pathIds)
		{
			this->scene->setActive(entity);
		}
	}
}

void RoomHandler::flipDoors(bool open)
{
	const uint32_t doorId = open ? this->openDoorMeshID : this->closedDoorMeshID;

	const Room& curRoom = this->rooms[activeIndex];

	if (curRoom.doorIds[0] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[0]).meshID = doorId; }
	if (curRoom.doorIds[1] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[1]).meshID = doorId; }
	if (curRoom.doorIds[2] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[2]).meshID = doorId; }
	if (curRoom.doorIds[3] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[3]).meshID = doorId; }

	if (this->nextIndex != -1)
	{
		const Room& nextRoom = this->rooms[nextIndex];

		if (nextRoom.doorIds[0] != -1) { this->scene->getComponent<MeshComponent>(nextRoom.doorIds[0]).meshID = doorId; }
		if (nextRoom.doorIds[1] != -1) { this->scene->getComponent<MeshComponent>(nextRoom.doorIds[1]).meshID = doorId; }
		if (nextRoom.doorIds[2] != -1) { this->scene->getComponent<MeshComponent>(nextRoom.doorIds[2]).meshID = doorId; }
		if (nextRoom.doorIds[3] != -1) { this->scene->getComponent<MeshComponent>(nextRoom.doorIds[3]).meshID = doorId; }
	}
}

glm::vec3 RoomHandler::snapToGrid(const glm::vec3& pos)
{
	return glm::vec3(
		std::floor(pos.x / TILE_WIDTH) * TILE_WIDTH, 
		0.f, 
		std::floor(pos.z / TILE_WIDTH) * TILE_WIDTH);
}

#ifdef _DEBUG
void RoomHandler::activateAll()
{
	for (size_t i = 0; i < this->rooms.size(); i++)
	{	
		for (const Entity& id : this->rooms[i].tileIds)
		{
			this->scene->setActive(id);
		}

		if (this->rooms[i].doorIds[0] != -1) { this->scene->setActive(this->rooms[i].doorIds[0]); }
		if (this->rooms[i].doorIds[1] != -1) { this->scene->setActive(this->rooms[i].doorIds[1]); }
		if (this->rooms[i].doorIds[2] != -1) { this->scene->setActive(this->rooms[i].doorIds[2]); }
		if (this->rooms[i].doorIds[3] != -1) { this->scene->setActive(this->rooms[i].doorIds[3]); }		
	}

	for (const Entity& entity : this->pathIds)
	{
		this->scene->setActive(entity);
	}
}
#endif // _DEBUG
