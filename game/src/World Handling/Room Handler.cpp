#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::TILE_WIDTH = 20.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 6;
#define PRINT_POS(pos) printf("(%d, %d, %d)\n", (int)pos.x, (int)pos.y, (int)pos.z)

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

	openDoorMeshID = resourceMan->addMesh("assets/models/doorOpen.obj");
	closedDoorMeshID = resourceMan->addMesh("assets/models/doorClosed.obj");
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

		if (ImGui::Checkbox("Show all rooms", &showAllRooms))
		{
			if (showAllRooms) { activateAll(); }
			else { setActiveRooms(); }
		}

		if (ImGui::Button("Reload")) 
		{
			generate();
		} 

		ImGui::Text("A: %d, N: %d, D: %d", activeIndex, nextIndex, curDoor, (int)insideDoor);
		ImGui::Text("Pos: (%d, %d, %d)", (int)playerPos.x, (int)playerPos.y, (int)playerPos.z);

		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();

#endif


	if (Input::isKeyPressed(Keys::E)) 
	{
		roomFinished = true;
		flipDoors(true);
		rooms[activeIndex].finished = true;
	}

#ifdef _DEBUG
	if (!showAllRooms)
	{
		checkRoom(activeIndex, playerPos);
		if (nextIndex != -1) checkRoom(nextIndex, playerPos);
	}
#else
	checkRoom(activeIndex, playerPos);
	if (nextIndex != -1) { checkRoom(nextIndex, playerPos); }
#endif // _DEBUG
}

void RoomHandler::generate()
{
	reset();

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
		this->roomGenerator.generateBorders(hasDoor);
		this->setExitPoints(i);

		// Create main tiles
		const int NUM_TILES = this->roomGenerator.getNrTiles();
		this->rooms[i].tileIds.resize(NUM_TILES);
		for (int j = 0; j < NUM_TILES; j++) 
		{
			this->rooms[i].tileIds[j] = createTileEntity(j, roomPos);
		}
	
		// Scale the room, RoomGenerator is dependant on a tile being 1x1
		scaleRoom(i, roomPos);
	}


	// Find exit pairs and create pathways
	{
		const int NUM_MAIN_ROOMS = roomLayout.getNumMainRooms();
		const std::vector<glm::ivec2>& connections = roomLayout.getConnections();
		exitPairs.resize(connections.size());

		for (size_t i = 0; i < connections.size(); i++)
		{
			const glm::ivec2& curCon = connections[i];
			std::pair<glm::vec3, glm::vec3>& pair = exitPairs[i];

			if (curCon.x < NUM_MAIN_ROOMS && curCon.y < NUM_MAIN_ROOMS)
			{
				// Vertical Connection
				// curCon.x = index of upper room
				// curCon.y = index of lower room

				pair.first = roomExitPoints[curCon.y].worldPositions[2];
				pair.first.z += TILE_WIDTH;
				
				pair.second = roomExitPoints[curCon.x].worldPositions[3];
				pair.second.z -= TILE_WIDTH;
			}
			else
			{
				// Horizontal connection
				// Conn[i].x = right room
				// Conn[i].y = left room

				pair.first = roomExitPoints[curCon.y].worldPositions[0];
				pair.first.x += TILE_WIDTH;

				pair.second = roomExitPoints[curCon.x].worldPositions[1];
				pair.second.x -= TILE_WIDTH;
			}
		}

		this->generatePathways();
	}
	
	this->roomLayout.clear();

#ifdef _DEBUG
	if (showAllRooms) { activateAll(); }
	else { setActiveRooms(); }
#else
	setActiveRooms();
#endif // _DEBUG

}

void RoomHandler::createDoors(int roomIndex)
{
	const RoomLayout::RoomData& curRoom = roomLayout.getRoom(roomIndex);
	const glm::vec3& roomPos = curRoom.position;

	Room& curRoomIds = this->rooms[roomIndex];

	if (curRoom.left != -1)
	{
		curRoomIds.doorIds[0] = this->createDoorEntity(-90.f);	
	}
	if (curRoom.right != -1) 
	{
		curRoomIds.doorIds[1] = this->createDoorEntity(90.f);
	}
	if (curRoom.up != -1) 
	{
		curRoomIds.doorIds[2] = this->createDoorEntity(180.f);
	}
	if (curRoom.down != -1) 
	{
		curRoomIds.doorIds[3] = this->createDoorEntity(0.f);
	}

	const float OFFSET = 1.5f;
	const glm::vec2 OFFSETS[4] = 
	{
		glm::vec2(OFFSET, 0.f),
		glm::vec2(-OFFSET, 0.f),
		glm::vec2(0.f, OFFSET),
		glm::vec2(0.f, -OFFSET)
	};

	const glm::vec2* doorTilePos = roomGenerator.getMinMaxPos();
	for (int i = 0; i < 4; i++)
	{
		hasDoor[i] = false;

		if (curRoomIds.doorIds[i] != -1)
		{
			hasDoor[i] = true;
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
	const glm::vec2* exitPositions = roomGenerator.getExitTilesPos();

	roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (rooms[roomIndex].doorIds[i] != -1)
		{ 
			roomExitPoints.back().worldPositions[i] = glm::vec3(exitPositions[i].x, 0.f, exitPositions[i].y);
		}
	}
}

void RoomHandler::generatePathways()
{	
	printf("#################################\n");
	glm::vec3 dV;
	glm::vec3 sV{};
	glm::vec3 curPos{};
	glm::vec3 borderPos{};
	Entity entity;

	for (size_t i = 0; i < this->exitPairs.size(); i++)
	{
		glm::vec3& p0 = this->exitPairs[i].first;
		glm::vec3& p1 = this->exitPairs[i].second;


		if ((p1.x - p0.x) < 0.f) { std::swap(p0, p1); }

		dV = p1 - p0;
		dV.z = dV.z > 0.f ? -dV.z : dV.z;

		curPos = p0;

		sV.x = p0.x < p1.x ? TILE_WIDTH : -TILE_WIDTH;
		sV.z = p0.z < p1.z ? TILE_WIDTH : -TILE_WIDTH;

		float err = dV.x + dV.z;
		float e2 = 0.f;
		printf("--------------------------\n");
		printf("P0: (%f, %f, %f), P1: (%f, %f, %f)\n", p0.x, p0.y, p0.z, p1.x, p1.y, p1.z);
		printf("dV: (%f, %f, %f), sV: (%f, %f, %f)\n\n", dV.x, dV.y, dV.z, sV.x, sV.y, sV.z);

		char cur = '\0';
		char old = '\0';
		int num = 0;
		size_t numBorders = pathIds.size();
		while(true)
		{
			pathIds.emplace_back(this->createPathEntity());
			this->scene->getComponent<Transform>(pathIds.back()).position = snapToGrid(curPos);
			
			auto a = snapToGrid(curPos);
			//printf("(%f, %f, %f), (%f, %f, %f)\n", curPos.x, curPos.y, curPos.z, a.x, a.y, a.z);

			if (glm::length(curPos - p0) >= glm::length(dV))
			{
				if (old == 'f')
				{
					printf("Creating...\n");
					borderPos = curPos;
					borderPos.x += sV.x;
					borderPos.z += sV.z;
					for (int j = 0; j < num; j++)
					{
						borderPos.x -= sV.x;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);

						borderPos.z -= sV.z * 2.f;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);
						borderPos.z += sV.z * 2.f;
					}
					//if (num == 1 || num == 0)
					//{
					//	borderPos.x -= sV.x;
					//	//borderPos.z += sV.z;
					//	entity = createPathBorderEntity(borderPos);
					//	this->pathIds.emplace_back(entity);
					//}
				}
				else
				{
					printf("Creating...\n");
					borderPos = curPos;
					borderPos.x += sV.x;
					for (int j = 0; j < num; j++)
					{
						borderPos.z -= sV.z;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);

						borderPos.x -= sV.x * 2.f;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);
						borderPos.x += sV.x * 2.f;
					}
					//if (num == 1 || num == 0)
					//{
					//	//borderPos.x -= sV.x;
					//	borderPos.z += sV.z;
					//	entity = createPathBorderEntity(borderPos);
					//	this->pathIds.emplace_back(entity);
					//}
				}
				break;
			}

			e2 = TILE_WIDTH * 2.f * err;
			//printf("e2: %f, err: %f\n", e2, err);

			borderPos = curPos;
			old = cur;

			if (e2 > dV.z)
			{
				err += dV.z;
				curPos.x += sV.x;
				printf("First\n");

				cur = 'f';
			}
			else if (e2 < dV.x)
			{
				err += dV.x;
				curPos.z += sV.z;
				printf("Second\n");

				cur = 's';
			}

#if 1
			num++;
			printf("cur: %c, old: %c\n", cur, old);
			//continue;
			if (old != cur && old != '\0')
			{
				printf("Hello?\n");
				printf("Num: %d\n", num);
				printf("(%f, %f, %f)\n", borderPos.x,borderPos.y,borderPos.z);

				if (old == 'f')
				{
					printf("Creating...\n");
					//borderPos = curPos;
					borderPos.z += sV.z;
					for (int j = 0; j < num - 1; j++)
					{
						borderPos.x -= sV.x;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);

						borderPos.z -= sV.z * 2.f;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);
						borderPos.z += sV.z * 2.f;
					}
					//if (num == 1 || num == 0)
					//{
					//	borderPos.x -= sV.x;
					//	//borderPos.z += sV.z;
					//	entity = createPathBorderEntity(borderPos);
					//	this->pathIds.emplace_back(entity);
					//}
				}
				else
				{
					printf("Creating...\n");
					//borderPos = curPos;
					borderPos.x += sV.x;
					for (int j = 0; j < num - 1; j++)
					{
						borderPos.z -= sV.z;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);

						borderPos.x -= sV.x * 2.f;
						entity = createPathBorderEntity(borderPos);
						this->pathIds.emplace_back(entity);
						borderPos.x += sV.x * 2.f;
					}
					//if (num == 1 || num == 0)
					//{
					//	//borderPos.x -= sV.x;
					//	borderPos.z += sV.z;
					//	entity = createPathBorderEntity(borderPos);
					//	this->pathIds.emplace_back(entity);
					//}
				}
				num = 0;
			}

#elif 1
			if (dirX && !oldDirX)
			{
				//printf("---Triggered ONE---\n");

				borderPos.x -= sV.x;
				
				Entity entity = createPathBorderEntity(snapToGrid(borderPos));
				this->pathIds.emplace_back(entity);
				//printf("(%f, %f, %f)\n", borderPos.x,borderPos.y,borderPos.z);
				numBorders++;
			}
			continue;
#else
			else if (!dirX)
			{	
				//printf("---Triggered TWO---\n");

				borderPos.x += sV.x;
				Entity entity = createPathBorderEntity(snapToGrid(borderPos));
				this->pathIds.emplace_back(entity);
				//printf("(%f, %f, %f)\n", borderPos.x,borderPos.y,borderPos.z);
				numBorders++;

				if (!oldDirX)
				{
					//printf("---Triggered TWO TWO---\n");
					borderPos.x -= sV.x * 2.f;
					entity = createPathBorderEntity(snapToGrid(borderPos));
					this->pathIds.emplace_back(entity);
					//printf("(%f, %f, %f)\n", borderPos.x,borderPos.y,borderPos.z);
					numBorders++;
				}
				/*else
				{
					printf("---Triggered TWO TWO TWO---\n");
					borderPos.x -= sV.x * 2.f;
					entity = createPathBorderEntity(snapToGrid(borderPos));
					this->pathIds.emplace_back(entity);
				}*/
			}
			else if (oldDirX)
			{
				//printf("---Triggered THREE---\n");

				borderPos = curPos;
				borderPos.z -= sV.z;

				Entity entity = createPathBorderEntity(snapToGrid(borderPos));
				this->pathIds.emplace_back(entity);
				//printf("(%f, %f, %f)\n", borderPos.x,borderPos.y,borderPos.z);
				numBorders++;

				borderPos = curPos;
				borderPos.x -= sV.x;
				borderPos.z += sV.z;

				entity = createPathBorderEntity(snapToGrid(borderPos));
				this->pathIds.emplace_back(entity);
				numBorders++;

				borderPos.x -= sV.x;
				entity = createPathBorderEntity(snapToGrid(borderPos));
				this->pathIds.emplace_back(entity);
				numBorders++;
			}
#endif
			
		}
		printf("num border: %zd\n", pathIds.size() - numBorders );
	}
}

void RoomHandler::scaleRoom(int index, const glm::vec3& roomPos)
{	
	Room& room = rooms[index];

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
	Entity pieceID = scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[this->roomGenerator.getTile(tileIndex).type];

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(RoomGenerator::DEFAULT_TILE_SCALE);

	transform.position = glm::vec3(
		roomGenerator.getTile(tileIndex).position.x,
		0.f,
		roomGenerator.getTile(tileIndex).position.y);

	return pieceID;
}

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = closedDoorMeshID;

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
	this->scene->getComponent<MeshComponent>(entity).meshID = tileMeshIds[Tile::Border];
	Transform& tra = this->scene->getComponent<Transform>(entity);
	tra.position = position;
	tra.scale *= RoomGenerator::DEFAULT_TILE_SCALE * TILE_WIDTH;
	PRINT_POS(tra.position);

	return entity;
}

void RoomHandler::checkRoom(int index, const glm::vec3& playerPos)
{
	insideDoor = false;

	for (int j = 0; j < 4; j++)
	{
		Entity id = rooms[index].doorIds[j];
		if (id == -1)
			continue;

		Transform& tra = scene->getComponent<Transform>(id);
		Box2D& box = scene->getComponent<Box2D>(id);

		if (box.colliding(tra.position, playerPos))
		{
			if (index == nextIndex && !rooms[index].finished)
			{
				activeIndex = index;
				nextIndex = -1;
				roomFinished = false;

				setActiveRooms();
				flipDoors(false);
			}

			else if (curDoor == -1 && roomFinished)
			{
				activeIndex = index;
				nextIndex = rooms[index].connectingIndex[j];
				curDoor = j;
				
				setActiveRooms();
				flipDoors(true);
			}
			insideDoor = true;
		}
	}
	if (index == activeIndex && !insideDoor)
	{ 
		curDoor = -1;
	}
}

void RoomHandler::reset()
{
	for (Room& room : rooms)
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

	for (int& id : pathIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	pathIds.clear();

	exitPairs.clear();
	roomExitPoints.clear();

	activeIndex = 0;
	nextIndex = -1;
}

void RoomHandler::setActiveRooms()
{
	const int num = (int)rooms.size();
	for (int i = 0; i < num; i++)
	{
		if (i == activeIndex || i == nextIndex)
		{
			for (Entity id : rooms[i].tileIds)
			{
				this->scene->setActive(id);
			}

			if (rooms[i].doorIds[0] != -1) { this->scene->setActive(rooms[i].doorIds[0]); }
			if (rooms[i].doorIds[1] != -1) { this->scene->setActive(rooms[i].doorIds[1]); }
			if (rooms[i].doorIds[2] != -1) { this->scene->setActive(rooms[i].doorIds[2]); }
			if (rooms[i].doorIds[3] != -1) { this->scene->setActive(rooms[i].doorIds[3]); }
		}
		else
		{
			for (Entity id : rooms[i].tileIds)
			{
				this->scene->setInactive(id);
			}

			if (rooms[i].doorIds[0] != -1) { this->scene->setInactive(rooms[i].doorIds[0]); }
			if (rooms[i].doorIds[1] != -1) { this->scene->setInactive(rooms[i].doorIds[1]); }
			if (rooms[i].doorIds[2] != -1) { this->scene->setInactive(rooms[i].doorIds[2]); }
			if (rooms[i].doorIds[3] != -1) { this->scene->setInactive(rooms[i].doorIds[3]); }
		}

	}
}

void RoomHandler::flipDoors(bool open)
{
	const uint32_t doorId = open ? openDoorMeshID : closedDoorMeshID;

	const Room& curRoom = rooms[activeIndex];

	if (curRoom.doorIds[0] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[0]).meshID = doorId; }
	if (curRoom.doorIds[1] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[1]).meshID = doorId; }
	if (curRoom.doorIds[2] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[2]).meshID = doorId; }
	if (curRoom.doorIds[3] != -1) { this->scene->getComponent<MeshComponent>(curRoom.doorIds[3]).meshID = doorId; }

	if (nextIndex != -1)
	{
		const Room& nextRoom = rooms[nextIndex];

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
	for (size_t i = 0; i < rooms.size(); i++)
	{	
		for (Entity id : rooms[i].tileIds)
		{
			this->scene->setActive(id);
		}

		if (rooms[i].doorIds[0] != -1) { this->scene->setActive(rooms[i].doorIds[0]); }
		if (rooms[i].doorIds[1] != -1) { this->scene->setActive(rooms[i].doorIds[1]); }
		if (rooms[i].doorIds[2] != -1) { this->scene->setActive(rooms[i].doorIds[2]); }
		if (rooms[i].doorIds[3] != -1) { this->scene->setActive(rooms[i].doorIds[3]); }		
	}
}
#endif // _DEBUG
