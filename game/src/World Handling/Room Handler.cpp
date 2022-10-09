#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

const float RoomHandler::OUT_OF_BOUNDS = 1000.f;
const float RoomHandler::ROOM_WIDTH = 200.f;

RoomHandler::RoomHandler()
	:activeRoomIdx(0), scene(nullptr), roomWidth(200.f), roomGridSize(0)
{
}

void RoomHandler::init(Scene* scene, int roomSize, int tileTypes)
{
	generator.init(roomSize, tileTypes);
	roomLayout.init(scene, glm::vec3(roomWidth));
	this->scene = scene;
	this->roomGridSize = (float)roomSize;
}

void RoomHandler::update()
{
	// Check if passed through door

#ifdef _DEBUG
	if (ImGui::Begin("Debug"))
	{
		ImGui::PushItemWidth(-100.f);
		ImGui::Text("Rooms");
		ImGui::Text("Num: %zd", rooms.size());
		if (ImGui::Button("Reload")) 
		{
			reload();
		}

#if !REAL_LAYOUT
		static int temp = activeRoomIdx;
		if (ImGui::InputInt("Current room", &temp, 1, 1))
		{
			temp = setActiveRoom(temp);
		}
#endif
		ImGui::Separator();
		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif

}

void RoomHandler::generate()
{
	roomLayout.generate();
	rooms.resize(roomLayout.getNumRooms());
	
	// Print
	for (int i = 0; i < roomLayout.getNumRooms(); i++)
	{
		Transform& tra = scene->getComponent<Transform>(roomLayout.getRoomID(i));
		Room& room = scene->getComponent<Room>(roomLayout.getRoomID(i));
		printf("%d: | (%d, %d, %d) | Left: %d, Right: %d, Up: %d, Down: %d\n", 
			i, (int)tra.position.x, (int)tra.position.y, (int)tra.position.z, room.left, room.right, room.up, room.down);
	}
	

	const float TILE_SCALE = roomWidth / (float)roomGridSize;
	
	for (int i = 0; i < roomLayout.getNumRooms(); i++)
	{
		//add tile enities
		generator.generateRoom();
		const int NUM_TILES = generator.getNrTiles();

		rooms[i].entities.resize(NUM_TILES);
		rooms[i].startPositions.resize(NUM_TILES);

#if REAL_LAYOUT
		glm::vec3 pos = scene->getComponent<Transform>(roomLayout.getRoomID(i)).position;
#else
		glm::vec3 pos = glm::vec3(0.f);
		pos.y = OUT_OF_BOUNDS;
#endif

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
				generator.getTile(j).position.x * TILE_SCALE + pos.x,
				pos.y,
				generator.getTile(j).position.y * TILE_SCALE + pos.z);

			rooms[i].startPositions[j] = transform.position;
			rooms[i].entities[j] = pieceID;
		}
		
		generator.clear();
	}

	roomLayout.clear();

#if !REAL_LAYOUT
	setActiveRoom(0);
#endif
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

void RoomHandler::reload()
{
	return;

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