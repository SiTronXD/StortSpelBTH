#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::OUT_OF_BOUNDS = 1000.f;
const float RoomHandler::ROOM_WIDTH = 200.f;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr), roomWidth(200.f), roomGridSize(0), numRooms(0)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, int roomSize, int tileTypes)
{
	this->roomGenerator.init(roomSize, tileTypes);
	this->roomLayout.init(scene, glm::vec3(this->roomWidth));
	
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->roomGridSize = (float)roomSize;

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
#ifdef _DEBUG
	this->tileIds.reserve(size_t(this->numRooms * 100));
	this->doors.reserve(size_t(this->numRooms * 4));
#endif // _DEBUG
	
	for (int i = 0; i < this->numRooms; i++)
	{
		//add tile enities
		this->roomGenerator.generateRoom();
		const int NUM_TILES = this->roomGenerator.getNrTiles();

		glm::vec3 pos = this->scene->getComponent<Transform>(this->roomLayout.getRoomID(i)).position;

		for (int j = 0; j < NUM_TILES; j++) 
		{
#ifdef _DEBUG
			this->createTileEntity(j, TILE_SCALE, pos);
#else
			tileIds.emplace_back(createTile(j, TILE_SCALE, pos));
#endif
		}
		
		this->createDoors(i, TILE_SCALE);

		this->roomGenerator.reset();
	}

	
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

void RoomHandler::createDoors(int roomIndex, float tileScale)
{
	Room& curRoom = scene->getComponent<Room>(roomLayout.getRoomID(roomIndex));
	const glm::vec3& curPos = scene->getComponent<Transform>(roomLayout.getRoomID(roomIndex)).position;

	const glm::vec2* doorTilePos = roomGenerator.getMinMaxPos();
	glm::vec2 doorWorldPos[4]{};
	int possibleDoors[4]{-1,-1,-1,-1};

	if (curRoom.left != -1) {
		possibleDoors[0] = this->createDoorEntity(-90.f);
		doorWorldPos[0] = doorTilePos[0] * tileScale;
	}
	if (curRoom.right != -1) {
		possibleDoors[1] = this->createDoorEntity(90.f);
		doorWorldPos[1] = doorTilePos[1] * tileScale;
	}
	if (curRoom.up != -1) {
		possibleDoors[2] = this->createDoorEntity(180.f);
		doorWorldPos[2] = doorTilePos[2] * tileScale;
	}
	if (curRoom.down != -1) {
		possibleDoors[3] = this->createDoorEntity(0.f);
		doorWorldPos[3] = doorTilePos[3] * tileScale;
	}

	for (int i = 0; i < 4; i++)
	{
		if (possibleDoors[i] != -1)
		{
#ifdef _DEBUG
			this->doors.emplace_back(possibleDoors[i]);
#endif
			this->scene->getComponent<Transform>(possibleDoors[i]).position = 
				glm::vec3(curPos.x + doorWorldPos[i].x, curPos.y, curPos.z + doorWorldPos[i].y);
		}
	}
}

void RoomHandler::createPathway(int direction)
{

}

#ifdef _DEBUG
void RoomHandler::reload()
{
	for (int& id : tileIds)
	{
		this->scene->removeEntity(id);
		id = -1;
	}

	for (int& id : doors)
	{
		this->scene->removeEntity(id);
		id = -1;
	}

	generate();
}
#endif // _DEBUG