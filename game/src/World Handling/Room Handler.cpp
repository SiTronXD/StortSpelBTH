#include "Room Handler.h"
#include "vengine/application/Scene.hpp"

RoomHandler::RoomHandler(Scene* scene, int roomSize, int tileTypes)
	:roomLayout(scene), scene(scene)
{
	generator.init(roomSize, tileTypes);

	generate();
}

void RoomHandler::update()
{
	// Temp
	if (Input::isKeyPressed(Keys::R))
	{
		generate();
	}

	// Check if passed through door
}


void RoomHandler::generate()
{
	roomLayout.setUpRooms();

	glm::vec3 roomPos(0.f);

	rooms.resize(roomLayout.getNumRooms());
	
	const float TILE_SCALE = 1.f;

	for (int i = 0; i < roomLayout.getNumRooms(); i++)
	{

		roomPos = scene->getComponent<Transform>(roomLayout.getRoomID(i)).position;

		//Room& curRoom = scene->getComponent<Room>(roomLayout.getRoomID(i));
		//printf("Room %d | Pos: (%f, %f, %f), Left: %d, Right: %d, Up: %d, Down: %d\n", 
		//	i, roomPos.x, roomPos.y, roomPos.z, curRoom.left, curRoom.right, curRoom.up, curRoom.down);



		//add tile enities
		generator.generateRoom();
		rooms[i].entities.resize(generator.getNrTiles());
		for (int j = 0; j < generator.getNrTiles(); j++) 
		{
			int pieceID = scene->createEntity();
			scene->setComponent<MeshComponent>(pieceID);
			MeshComponent& mesh = scene->getComponent<MeshComponent>(pieceID);

			std::string path = "room_piece_" + std::to_string(generator.getTile(j).type) + ".obj";
			memcpy(mesh.filePath, path.c_str(), sizeof(mesh.filePath));

			Transform& transform = scene->getComponent<Transform>(pieceID);
			transform.position = glm::vec3(
				generator.getTile(j).position.x * TILE_SCALE + roomPos.x,
				0.f + roomPos.y,
				generator.getTile(j).position.y * TILE_SCALE + roomPos.z);

			transform.scale = glm::vec3(0.04f) * TILE_SCALE;

			rooms[i].entities[j] = pieceID;
		
		}
		generator.clear();
	}
}

