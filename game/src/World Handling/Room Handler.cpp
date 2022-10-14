#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::OUT_OF_BOUNDS = 1000.f;
const float RoomHandler::ROOM_WIDTH = 200.f;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), roomWidth(200.f), roomGridSize(0), numRooms(0),
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
	this->tileWidth = roomWidth / (float)roomSize;

	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.init(scene, roomWidth + tileWidth * numTilesInbetween);
	

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
	const float TILE_SCALE = this->roomWidth / (float)this->roomGridSize;

	this->roomLayout.generate();
	this->numRooms = this->roomLayout.getNumRooms();
	this->exitPoints.reserve(this->numRooms);
#ifdef _DEBUG
	this->tileIds.reserve(size_t(this->numRooms * 100));
	this->doors.reserve(size_t(this->numRooms * 4));
#endif // _DEBUG

	
	
	for (int i = 0; i < this->numRooms; i++)
	{
		//add tile enities
		this->roomGenerator.generateRoom();
		this->createDoors(i, TILE_SCALE);
		this->roomGenerator.generateBorders(hasDoor);

		const glm::vec3& roomPos = this->scene->getComponent<Transform>(this->roomLayout.getRoomID(i)).position;
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

	this->connections.reserve((size_t)this->roomLayout.getNumMainRooms() * 2);
	for (int i = 0; i < this->roomLayout.getNumMainRooms(); i++)
	{
		Room& currentRoom = this->scene->getComponent<Room>(this->roomLayout.getRoomID(i));

		if (currentRoom.left != -1)
		{
			this->connections.emplace_back();
			this->connections.back().index1 = i;
			this->connections.back().index2 = currentRoom.left;
		}
		if (currentRoom.right != -1)
		{
			this->connections.emplace_back();
			this->connections.back().index1 = i;
			this->connections.back().index2 = currentRoom.right;
		}
	}

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

	//this->scene->getComponent<MeshComponent>(id).meshID = this->tileMeshIds[Tile::OneXOne];
	
	// temp
	this->scene->getComponent<MeshComponent>(id).meshID = doorMeshId;
	this->scene->getComponent<Transform>(id).scale.y = 100.f;

	return id;
}

void RoomHandler::createDoors(int roomIndex, float tileScale)
{
	Room& curRoom = scene->getComponent<Room>(roomLayout.getRoomID(roomIndex));
	const glm::vec3& curPos = scene->getComponent<Transform>(roomLayout.getRoomID(roomIndex)).position;

	const glm::vec2* doorTilePos = roomGenerator.getMinMaxPos();
	int possibleDoors[4]{-1,-1,-1,-1};

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

	exitPoints.emplace_back();

	for (int i = 0; i < 4; i++)
	{
		hasDoor[i] = false;

		if (possibleDoors[i] != -1)
		{
#ifdef _DEBUG
			this->doors.emplace_back(possibleDoors[i]);
#endif
			hasDoor[i] = true;
			exitPoints.back().worldPositions[i] = new glm::vec2(doorTilePos[i]* tileScale);

			this->scene->getComponent<Transform>(possibleDoors[i]).position = 
				glm::vec3(
					curPos.x + (*exitPoints.back().worldPositions[i]).x,
					curPos.y, 
					curPos.z + (*exitPoints.back().worldPositions[i]).y);
		}
	}
}
//
//void RoomHandler::createPathways(float tileScale)
//{
//	// +x, -x, +z, -y
//	const glm::vec2* exitTiles = this->roomGenerator.getExitTiles();
//	
//	int indicies[4]{-1, -1, -1, -1};
//	
//	indicies[0] = currentRoom.left	!= -1 ? 0 : -1;
//	indicies[1] = currentRoom.right != -1 ? 0 : -1;
//	indicies[2] = currentRoom.up	!= -1 ? 0 : -1;
//	indicies[3] = currentRoom.down	!= -1 ? 0 : -1;
//	
//	for (int i = 0; i < 4; i++)
//	{
//		for (int i = 0; i < k; i++)
//		{
//
//		}
//		int id = this->createPathEntity();
//		Transform& transform = this->scene->getComponent<Transform>(id);
//		transform.position.x = exitTiles[0].x * tileScale + roomPos.x;
//		transform.position.y = roomPos.y;
//		transform.position.z = exitTiles[0].y * tileScale + roomPos.z;
//
//#ifdef _DEBUG
//		this->tileIds.emplace_back(id);
//#endif // _DEBUG
//	}
//
//
//	if (currentRoom.right != -1)
//	{
//		int id = this->createPathEntity();
//		Transform& transform = this->scene->getComponent<Transform>(id);
//		transform.position.x = exitTiles[1].x * tileScale + roomPos.x;
//		transform.position.y = roomPos.y;
//		transform.position.z = exitTiles[1].y * tileScale + roomPos.z;
//	}
//	if (currentRoom.up != -1)
//	{
////		for (int i = 0; i < numTilesInbetween; i++)
////		{
////			int id = this->createPathEntity();
////			Transform& transform = this->scene->getComponent<Transform>(id);
////			transform.position.x = exitTiles[0].x * tileScale + roomPos.x;
////			transform.position.y = roomPos.y;
////			transform.position.z = (exitTiles[0].y * tileScale) + tileWidth * i + roomPos.z;
////
////#ifdef _DEBUG
////			this->tileIds.emplace_back(id);
////#endif // _DEBUG
////
////		}
//
//		int id = this->createPathEntity();
//		Transform& transform = this->scene->getComponent<Transform>(id);
//		transform.position.x = exitTiles[2].x * tileScale + roomPos.x;
//		transform.position.y = roomPos.y;
//		transform.position.z = exitTiles[2].y * tileScale + roomPos.z;
//
//	}
//	if (currentRoom.down != -1)
//	{
//		int id = this->createPathEntity();
//		Transform& transform = this->scene->getComponent<Transform>(id);
//		transform.position.x = exitTiles[3].x * tileScale + roomPos.x;
//		transform.position.y = roomPos.y;
//		transform.position.z = exitTiles[3].y * tileScale + roomPos.z;
//	}
//}

#ifdef _DEBUG
void RoomHandler::reload()
{
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

	generate();
}
#endif // _DEBUG