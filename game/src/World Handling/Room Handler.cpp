#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::ROOM_WIDTH = 200.f;

/*
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
	EXIT POINTS MESSED UP WHEN DOOR GENERATED ON THE EDGE
*/

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), roomGridSize(0), numRooms(0),
	numTilesInbetween(3)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->roomGridSize = (float)roomSize;
	this->tileWidth = ROOM_WIDTH / (float)roomSize;

	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.init(scene, ROOM_WIDTH + tileWidth * numTilesInbetween);
	

	// Border pieces not counted in tileTypes
	for (int i = 0; i < tileTypes + 1; i++)
	{
		// Tile types in RoomGenerator ranges from 1-tileTypes
		uint32_t id = resourceMan->addMesh("assets/models/room_piece_" + std::to_string(i) + ".obj");
		this->tileMeshIds[Tile::Type(i)] = id;
	}
	doorMeshId = resourceMan->addMesh("assets/models/door.obj");
}

void RoomHandler::update()
{
	// Check if passed through door

#ifdef _DEBUG
	if (ImGui::Begin("Debug"))
	{
		ImGui::PushItemWidth(-100.f);
		ImGui::Text("Rooms");
		ImGui::Text("Num: %zd", this->numRooms);
		if (ImGui::Button("Reload")) 
		{
			reload();
		}

		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif

}

void RoomHandler::generate()
{
	const float TILE_SCALE = this->ROOM_WIDTH / (float)this->roomGridSize;

	this->roomLayout.generate();
	this->numRooms = this->roomLayout.getNumRooms();
	this->roomExitPoints.clear();
	this->exitPairs.clear();

#ifdef _DEBUG
	this->tileIds.clear();
	this->doors.clear();
	this->pathIds.clear();
#endif // _DEBUG

	for (int i = 0; i < this->numRooms; i++)
	{
		memset(possibleDoors, -1, sizeof(possibleDoors));
		const glm::vec3& roomPos = this->scene->getComponent<Transform>(this->roomLayout.getRoomID(i)).position;

		//add tile enities
		this->roomGenerator.generateRoom();
		this->createDoors(i, TILE_SCALE);
		this->roomGenerator.generateBorders(hasDoor);
		this->createConnectionPoint(TILE_SCALE, roomPos);

		const int NUM_TILES = this->roomGenerator.getNrTiles();
		for (int j = 0; j < NUM_TILES; j++) 
		{
#ifdef _DEBUG
			tileIds.emplace_back(createTileEntity(j, TILE_SCALE, roomPos));
#else
			this->createTileEntity(j, TILE_SCALE, roomPos);
#endif
		}
	
		this->roomGenerator.reset();
	}

	int numMainRooms = roomLayout.getNumMainRooms();
	std::vector<glm::ivec2>& connections = roomLayout.getConnections();
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

		auto a = exitPairs.back().first;
		auto b = exitPairs.back().second;
		printf("connection: %zd | pos: (%f, %f, %f) - (%f, %f, %f)\n", i, a.x, a.y, a.z, b.x, b.y, b.z);
	}

	this->createPathways(TILE_SCALE);

	
/*#ifdef _DEBUG
	for (int i = 0; i < exitPairs.size(); i++)
	{
		int id1 = this->createPathEntity();
		int id2 = this->createPathEntity();

		this->scene->getComponent<MeshComponent>(id1).meshID = doorMeshId;
		this->scene->getComponent<MeshComponent>(id2).meshID = doorMeshId;

		Transform& tra1 = this->scene->getComponent<Transform>(id1);
		Transform& tra2 = this->scene->getComponent<Transform>(id2); 

		tra1.position = exitPairs[i].first;
		tra2.position = exitPairs[i].second;

		tra1.scale.y = 5.f;
		tra2.scale.y = 5.f;

		connectionsIds.emplace_back(id1);
		connectionsIds.emplace_back(id2);
	}
#endif*/

	/*this->exitPairs.reserve((size_t)this->roomLayout.getNumMainRooms() * 2);
	for (int i = 0; i < this->roomLayout.getNumMainRooms(); i++)
	{
		Room& currentRoom = this->scene->getComponent<Room>(this->roomLayout.getRoomID(i));

		if (currentRoom.left != -1)
		{
			this->exitPairs.emplace_back();
			this->exitPairs.back().index1 = i;
			this->exitPairs.back().index2 = currentRoom.left;
		}
		if (currentRoom.right != -1)
		{
			this->exitPairs.emplace_back();
			this->exitPairs.back().index1 = i;
			this->exitPairs.back().index2 = currentRoom.right;
		}
	}*/

	//this->createPathways(TILE_SCALE);
	
	// Print
	/*for (int i = 0; i < roomLayout.getNumRooms(); i++)
	{
		Transform& tra = scene->getComponent<Transform>(roomLayout.getRoomID(i));
		Room& room = scene->getComponent<Room>(roomLayout.getRoomID(i));
		printf("%d: | (%d, %d, %d) | Left: %d, Right: %d, Up: %d, Down: %d\n", 
			i, (int)tra.position.x, (int)tra.position.y, (int)tra.position.z, room.left, room.right, room.up, room.down);
	}*/

	this->roomLayout.clear();
}

int RoomHandler::createTileEntity(int tileIndex, float tileScale, const glm::vec3& roomPos)
{
	int pieceID = scene->createEntity();
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

int RoomHandler::createDoorEntity(float yRotation)
{
	int entity = scene->createEntity();

	this->scene->setComponent<MeshComponent>(entity);
	this->scene->getComponent<MeshComponent>(entity).meshID = doorMeshId;

	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;

	return entity;
}

int RoomHandler::createPathEntity()
{
	int id = this->scene->createEntity();
	this->scene->setComponent<MeshComponent>(id);

	this->scene->getComponent<MeshComponent>(id).meshID = this->tileMeshIds[Tile::OneXOne];
	
	// temp
	//this->scene->getComponent<MeshComponent>(id).meshID = doorMeshId;
	//this->scene->getComponent<Transform>(id).scale.y = 100.f;

	return id;
}

void RoomHandler::createDoors(int roomIndex, float tileScale)
{
	Room& curRoom = scene->getComponent<Room>(roomLayout.getRoomID(roomIndex));
	const glm::vec3& curPos = scene->getComponent<Transform>(roomLayout.getRoomID(roomIndex)).position;

	const glm::vec2* doorTilePos = roomGenerator.getMinMaxPos();

	if (curRoom.left != -1) {
		possibleDoors[0] = this->createDoorEntity(-90.f);	
	}
	if (curRoom.right != -1) {
		possibleDoors[1] = this->createDoorEntity(90.f);
	}
	if (curRoom.up != -1) {
		possibleDoors[2] = this->createDoorEntity(180.f);
	}
	if (curRoom.down != -1) {
		possibleDoors[3] = this->createDoorEntity(0.f);
	}

	for (int i = 0; i < 4; i++)
	{
		hasDoor[i] = false;

		if (possibleDoors[i] != -1)
		{
#ifdef _DEBUG
			this->doors.emplace_back(possibleDoors[i]);
#endif

			hasDoor[i] = true;
			glm::vec3 pos(doorTilePos[i].x, 0.f, doorTilePos[i].y);

			this->scene->getComponent<Transform>(possibleDoors[i]).position = pos * tileScale + curPos;
		}
	}
}

void RoomHandler::createConnectionPoint(float tileScale, const glm::vec3& roomPos)
{
	const glm::vec2* exitPositions = roomGenerator.getExitTiles();

	roomExitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		if (possibleDoors[i] != -1)
		{ 
			roomExitPoints.back().worldPositions[i] = 
				glm::vec3(exitPositions[i].x, 0.f, exitPositions[i].y) * tileScale + roomPos;;
		}
	}
}

void RoomHandler::createPathways(float tileScale)
{	
	glm::vec3 dV;
	glm::vec3 sV{};
	glm::vec3 tilePos, gridPos;
	glm::vec3 curPos{};

	for (size_t i = 0; i < this->exitPairs.size(); i++)
	{
		glm::vec3& p0 = this->exitPairs[i].first;
		glm::vec3& p1 = this->exitPairs[i].second;

		if ((p1.x - p0.x) < 0.f)
		{
			std::swap(p0, p1);
		}

		dV = p1 - p0;
		dV.z = dV.z > 0.f ? -dV.z : dV.z;

		curPos = p0;

		// fix
		sV.x = p0.x < p1.x ? tileWidth : -tileWidth;
		sV.z = p0.z < p1.z ? tileWidth : -tileWidth;

		float err = dV.x + dV.z;
		float e2;

		for (;;)
		{
			gridPos = curPos / tileWidth;
			tilePos = glm::vec3(std::floor(gridPos.x) * tileWidth, 0.f, std::floor(gridPos.z) * tileWidth);

			pathIds.emplace_back(this->createPathEntity());
			this->scene->getComponent<Transform>(pathIds.back()).position = tilePos;
			this->scene->getComponent<Transform>(pathIds.back()).scale = glm::vec3(1.f);
			
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

#ifdef _DEBUG
void RoomHandler::reload()
{
	system("CLS");

	for (int& id : tileIds)
	{
		this->scene->removeEntity(id);
	}
	tileIds.clear();

	for (int& id : doors)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	doors.clear();

	for (int& id : connectionsIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	connectionsIds.clear();

	for (int& id : pathIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}
	pathIds.clear();


	generate();
}
#endif // _DEBUG