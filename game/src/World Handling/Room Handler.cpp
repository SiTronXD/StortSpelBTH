#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::ROOM_WIDTH = 200.f;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), roomGridSize(0),
	tileWidth(0.f), hasDoor{},
	activeIndex(0), nextIndex(-1),
	openDoorMeshID(0), closedDoorMeshID(0)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->roomGridSize = roomSize;
	this->tileWidth = ROOM_WIDTH / (float)roomSize;

	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.setRoomDistance(ROOM_WIDTH + tileWidth * TILES_BETWEEN_ROOMS);	

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
		if (ImGui::Button("Reload")) 
		{
			generate();
		} 
		ImGui::SameLine();

		ImGui::Text("A: %d, N: %d, D: %d", activeIndex, nextIndex, curDoor, (int)insideDoor);

		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();


	if (Input::isKeyPressed(Keys::E)) 
	{
		roomFinished = true;
		flipDoors(true);
	}
#endif

	for (size_t i = 0; i < rooms.size(); i++)
	{
		if (i != activeIndex && i != nextIndex)
				continue;

		insideDoor = false;

		for (int j = 0; j < 4; j++)
		{
			Entity id = rooms[i].doorIds[j];
			if (id == -1)
				continue;

			Transform& tra = scene->getComponent<Transform>(id);
			Box2D& box = scene->getComponent<Box2D>(id);

			if (box.colliding(tra.position, playerPos))
			{
				if (i == nextIndex)
				{
					activeIndex = i;
					nextIndex = -1;
					roomFinished = false;
					
					printf("HELO! - A: %d, N: %d, D: %d\n", activeIndex, nextIndex, curDoor);
					setActiveRooms();
					flipDoors(false);
				}

				else if (curDoor == -1 && roomFinished)
				{
					activeIndex = i;
					nextIndex = rooms[i].connectingIndex[j];
					curDoor = j;
					printf("BAI! - A: %d, N: %d, D: %d\n", activeIndex, nextIndex, curDoor);
					setActiveRooms();
					flipDoors(true);
				}
				insideDoor = true;
			}
		}
		if (i == activeIndex && !insideDoor) { curDoor = -1; }
	}
}

void RoomHandler::generate()
{
	reset();

	const float TILE_SCALE = this->ROOM_WIDTH / (float)this->roomGridSize;

	this->roomLayout.generate();
	const int numRooms = this->roomLayout.getNumRooms();
	this->rooms.resize(numRooms);

	for (int i = 0; i < numRooms; i++)
	{
		const RoomLayout::RoomData& roomRef = this->roomLayout.getRoom(i);
		const glm::vec3& roomPos = roomRef.position;

		this->rooms[i].connectingIndex[0] = roomRef.left;
		this->rooms[i].connectingIndex[1] = roomRef.right;
		this->rooms[i].connectingIndex[2] = roomRef.up;
		this->rooms[i].connectingIndex[3] = roomRef.down;


		//add tile enities
		this->roomGenerator.generateRoom();
		this->createDoors(i, TILE_SCALE);
		this->roomGenerator.generateBorders(hasDoor);
		this->createConnectionPoint(i, TILE_SCALE, roomPos);

		const int NUM_TILES = this->roomGenerator.getNrTiles();
		this->rooms[i].tileIds.resize(NUM_TILES);

		for (int j = 0; j < NUM_TILES; j++) 
		{
			this->rooms[i].tileIds[j] = createTileEntity(j, TILE_SCALE, roomPos);
		}
	
		this->roomGenerator.reset();
	}

	// Find exitPairs
	{
		int numMainRooms = roomLayout.getNumMainRooms();
		const std::vector<glm::ivec2>& connections = roomLayout.getConnections();
		exitPairs.reserve(connections.size());

		for (size_t i = 0; i < connections.size(); i++)
		{
			const glm::ivec2& curCon = connections[i];
			exitPairs.emplace_back();

			if (curCon.x < numMainRooms && curCon.y < numMainRooms)
			{
				// Vertical Connection
				// curCon.x = index of upper room
				// curCon.y = index of lower room

				exitPairs.back().first = roomExitPoints[curCon.y].worldPositions[2];
				exitPairs.back().first.z += tileWidth;

				exitPairs.back().second = roomExitPoints[curCon.x].worldPositions[3];
				exitPairs.back().second.z -= tileWidth;
			}
			else
			{
				// Horizontal connection
				// Conn[i].x = right room
				// Conn[i].y = left room

				exitPairs.back().first = roomExitPoints[curCon.y].worldPositions[0];
				exitPairs.back().first.x += tileWidth;

				exitPairs.back().second = roomExitPoints[curCon.x].worldPositions[1];
				exitPairs.back().second.x -= tileWidth;
			}
		}
	}
	

	this->createPathways(TILE_SCALE);

	this->roomLayout.clear();

	setActiveRooms();
}

Entity RoomHandler::createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos)
{
	Entity pieceID = scene->createEntity();
	this->scene->setComponent<MeshComponent>(pieceID);
	this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[this->roomGenerator.getTile(tileIndex).type];

	if (roomGenerator.getTile(tileIndex).type == Tile::Border)
		this->scene->getComponent<MeshComponent>(pieceID).meshID = (int)this->tileMeshIds[Tile::Border];

	Transform& transform = this->scene->getComponent<Transform>(pieceID);
	transform.scale = glm::vec3(0.04f) * tileScale;

	transform.position = glm::vec3(
		roomGenerator.getTile(tileIndex).position.x * tileScale + roomPos.x,
		roomPos.y,
		roomGenerator.getTile(tileIndex).position.y * tileScale + roomPos.z);

	return pieceID;
}

Entity RoomHandler::createDoorEntity(float yRotation, const glm::vec3& offset)
{
	Entity entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = closedDoorMeshID;

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;
	transform.position += offset;

	return entity;
}

Entity RoomHandler::createPathEntity(float tileScale)
{
	Entity id = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(id);
	this->scene->getComponent<MeshComponent>(id).meshID = this->tileMeshIds[Tile::OneXOne];
	
	this->scene->getComponent<Transform>(id).scale = glm::vec3(0.04f) * tileScale;

	return id;
}

void RoomHandler::createDoors(int roomIndex, float tileScale)
{
	const RoomLayout::RoomData& curRoom = roomLayout.getRoom(roomIndex);
	const glm::vec3& roomPos = curRoom.position;

	Room& curRoomIds = this->rooms[roomIndex];

	const glm::vec2* doorTilePos = roomGenerator.getMinMaxPos();
	const float OFFSET = tileWidth * 1.5f;

	if (curRoom.left != -1)
	{
		curRoomIds.doorIds[0] = this->createDoorEntity(-90.f, glm::vec3(OFFSET , 0.f, 0.f));	
	}
	if (curRoom.right != -1) 
	{
		curRoomIds.doorIds[1] = this->createDoorEntity(90.f, glm::vec3(-OFFSET, 0.f, 0.f));
	}
	if (curRoom.up != -1) 
	{
		curRoomIds.doorIds[2] = this->createDoorEntity(180.f, glm::vec3(0.f, 0.f, OFFSET));
	}
	if (curRoom.down != -1) 
	{
		curRoomIds.doorIds[3] = this->createDoorEntity(0.f, glm::vec3(0.f, 0.f, -OFFSET));
	}

	for (int i = 0; i < 4; i++)
	{
		hasDoor[i] = false;

		if (curRoomIds.doorIds[i] != -1)
		{
			hasDoor[i] = true;

			glm::vec3 pos(doorTilePos[i].x, 0.f, doorTilePos[i].y);
			this->scene->getComponent<Transform>(curRoomIds.doorIds[i]).position += pos * tileScale + roomPos;

			this->scene->setComponent<Box2D>(curRoomIds.doorIds[i]);
			Box2D& trigger = this->scene->getComponent<Box2D>(curRoomIds.doorIds[i]);
			trigger.extents.x = tileWidth * 0.2f;
			trigger.extents.y = tileWidth * 0.2f;
		}
	}
}

void RoomHandler::createConnectionPoint(int roomIndex, float tileScale, const glm::vec3& roomPos)
{
	const glm::vec2* exitPositions = roomGenerator.getExitTilesPos();

	roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (rooms[roomIndex].doorIds[i] != -1)
		{ 
			roomExitPoints.back().worldPositions[i] = 
				glm::vec3(exitPositions[i].x, 0.f, exitPositions[i].y) * tileScale + roomPos;
		}
	}
}

void RoomHandler::createPathways(float tileScale)
{	
	glm::vec3 dV;
	glm::vec3 sV{};
	glm::vec3 curPos{};

	for (size_t i = 0; i < this->exitPairs.size(); i++)
	{
		glm::vec3& p0 = this->exitPairs[i].first;
		glm::vec3& p1 = this->exitPairs[i].second;

		if ((p1.x - p0.x) < 0.f) { std::swap(p0, p1); }

		dV = p1 - p0;
		dV.z = dV.z > 0.f ? -dV.z : dV.z;

		curPos = p0;

		sV.x = p0.x < p1.x ? tileWidth : -tileWidth;
		sV.z = p0.z < p1.z ? tileWidth : -tileWidth;

		float err = dV.x + dV.z;
		float e2;

		while(true)
		{
			pathIds.emplace_back(this->createPathEntity(tileScale));
			this->scene->getComponent<Transform>(pathIds.back()).position = snapToGrid(curPos);
			
			if (glm::length(curPos - p0) >= glm::length(dV))
			{
				break;
			}

			e2 = tileWidth * 2.f * err;

			if (e2 > dV.z)
			{
				err += dV.z;
				curPos.x += sV.x;
			}
			else if (e2 < dV.x)
			{
				err += dV.x;
				curPos.z += sV.z;
			}
		}
	}
}

glm::vec3 RoomHandler::snapToGrid(const glm::vec3& pos)
{
	return glm::vec3(
		std::floor(pos.x / tileWidth) * tileWidth, 
		0.f, 
		std::floor(pos.z / tileWidth) * tileWidth);
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

	if (curRoom.doorIds[0] != -1) this->scene->getComponent<MeshComponent>(curRoom.doorIds[0]).meshID = doorId;
	if (curRoom.doorIds[1] != -1) this->scene->getComponent<MeshComponent>(curRoom.doorIds[1]).meshID = doorId;
	if (curRoom.doorIds[2] != -1) this->scene->getComponent<MeshComponent>(curRoom.doorIds[2]).meshID = doorId;
	if (curRoom.doorIds[3] != -1) this->scene->getComponent<MeshComponent>(curRoom.doorIds[3]).meshID = doorId;

	if (nextIndex != -1)
	{
		const Room& nextRoom = rooms[nextIndex];

		if (nextRoom.doorIds[0] != -1) this->scene->getComponent<MeshComponent>(nextRoom.doorIds[0]).meshID = doorId;
		if (nextRoom.doorIds[1] != -1) this->scene->getComponent<MeshComponent>(nextRoom.doorIds[1]).meshID = doorId;
		if (nextRoom.doorIds[2] != -1) this->scene->getComponent<MeshComponent>(nextRoom.doorIds[2]).meshID = doorId;
		if (nextRoom.doorIds[3] != -1) this->scene->getComponent<MeshComponent>(nextRoom.doorIds[3]).meshID = doorId;
	}
}