#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

RoomHandler::RoomHandler()
	:activeRoomIdx(0), scene(nullptr)
{
}

void RoomHandler::init(Scene* scene, int roomSize, int tileTypes)
{
	generator.init(roomSize, tileTypes);
	roomLayout.setScene(scene);
	this->scene = scene;
}

void RoomHandler::update()
{
	// Check if passed through door

	if (ImGui::Begin("Rooms"))
	{
		static int temp = activeRoomIdx;
		if (ImGui::InputInt("Current room: %d", &temp, 1, 1))
			temp = setActiveRoom(temp);
	}
	ImGui::End();
}

void RoomHandler::reload()
{
	for (RoomStorage& room : rooms)
	{
		for (int& id : room.entities)
		{
			scene->removeEntity(id);
			id = -1;
		}
	}

	generate();
}

void RoomHandler::generate()
{
	roomLayout.generate();
	rooms.resize(roomLayout.getNumRooms());
	
	const float TILE_SCALE = 25.f;

	for (int i = 0; i < roomLayout.getNumRooms(); i++)
	{
		//add tile enities
		generator.generateRoom();
		const int NUM_TILES = generator.getNrTiles();

		rooms[i].entities.resize(NUM_TILES);
		rooms[i].startPositions.resize(NUM_TILES);

		for (int j = 0; j < NUM_TILES; j++) 
		{
			int pieceID = scene->createEntity();
			scene->setComponent<MeshComponent>(pieceID);
			MeshComponent& mesh = scene->getComponent<MeshComponent>(pieceID);

			std::string path = "room_piece_" + std::to_string(generator.getTile(j).type) + ".obj";
			memcpy(mesh.filePath, path.c_str(), sizeof(mesh.filePath));

			Transform& transform = scene->getComponent<Transform>(pieceID);
			transform.scale = glm::vec3(0.04f) * TILE_SCALE;
			transform.position = glm::vec3(
				generator.getTile(j).position.x * TILE_SCALE,
				OUT_OF_BOUNDS,
				generator.getTile(j).position.y * TILE_SCALE);

			rooms[i].startPositions[j] = transform.position;
			rooms[i].entities[j] = pieceID;		
		}

		generator.clear();
	}

	roomLayout.clear();

	setActiveRoom(0);
}

int RoomHandler::setActiveRoom(int index)
{
 	if ((index < 0 || index >= (int)rooms.size()) || (activeRoomIdx < 0 || activeRoomIdx >= (int)rooms.size()))
	{
		Log::warning("Invalid room index, forcing to room 0");
		setActiveRoom(0);
		return 0;
	}

	// Move away old room
	for (int entity : rooms[activeRoomIdx].entities)
	{
		scene->getComponent<Transform>(entity).position.y = OUT_OF_BOUNDS;
	}

	// Move back new room
	for (int entity : rooms[index].entities)
	{
		scene->getComponent<Transform>(entity).position.y = 0.f;
	}

	activeRoomIdx = index;
	return activeRoomIdx;
}