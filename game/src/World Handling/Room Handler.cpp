#include "Room Handler.h"
#include "vengine/application/Scene.hpp"
#include "vengine/dev/Random.hpp"
#include "vengine/graphics/DebugRenderer.hpp"

const float RoomHandler::TILE_WIDTH = 25.f;
const uint32_t RoomHandler::TILES_BETWEEN_ROOMS = 5;
const uint32_t RoomHandler::NUM_BORDER = 1;
const uint32_t RoomHandler::NUM_ONE_X_ONE = 3;
const uint32_t RoomHandler::NUM_ONE_X_TWO = 1;
const uint32_t RoomHandler::NUM_TWO_X_TWO = 1;
const uint32_t RoomHandler::DECO_ENTITY_CHANCE = 30;

RoomHandler::RoomHandler()
	:scene(nullptr), resourceMan(nullptr),
	activeIndex(0), nextIndex(-1), floor(-1),
	doorMeshID(0), tileFloorMeshId(0), innerBorderMesh(0)
	, rockFenceMeshId(0), rockMeshId(0), random(), useMeshes(true)
{
}

RoomHandler::~RoomHandler()
{
}

void RoomHandler::init(Scene* scene, ResourceManager* resourceMan, bool useMeshes)
{
	this->scene = scene;
	this->resourceMan = resourceMan;
	this->useMeshes = useMeshes;

	if (this->useMeshes)
	{
		this->borderMeshIds.resize(NUM_BORDER);
		for (uint32_t i = 0; i < NUM_BORDER; i++)
		{
			this->borderMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/Border/" + std::to_string(i + 1u) + ".obj");
		}
		this->innerBorderMesh = resourceMan->addMesh("assets/models/Tiles/Border/innerBorder.obj");
	}

	if (this->useMeshes)
	{
		this->oneXOneMeshIds.resize(NUM_ONE_X_ONE);
		for (uint32_t i = 0; i < NUM_ONE_X_ONE; i++)
		{
			this->oneXOneMeshIds[i] = resourceMan->addMesh("assets/models/Tiles/OneXOne/" + std::to_string(i + 1u) + ".obj");
		}
	}

	this->oneXTwoMeshIds.resize(NUM_ONE_X_TWO);
	for (uint32_t i = 0; i < NUM_ONE_X_TWO; i++)
	{
		if (this->useMeshes)
		{
			this->oneXTwoMeshIds[i].first = resourceMan->addMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
		}
		const uint32_t collId = resourceMan->addCollisionShapeFromMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
		this->oneXTwoMeshIds[i].second = resourceMan->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;
	}

	this->twoXTwoMeshIds.resize(NUM_TWO_X_TWO);
	for (uint32_t i = 0; i < NUM_TWO_X_TWO; i++)
	{
		if (this->useMeshes)
		{
			this->twoXTwoMeshIds[i].first = resourceMan->addMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
		}
		const uint32_t collId = resourceMan->addCollisionShapeFromMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
		this->twoXTwoMeshIds[i].second = resourceMan->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;

	}
	if (this->useMeshes)
	{
		this->doorMeshID = resourceMan->addMesh("assets/models/door.obj");
		this->rockMeshId = resourceMan->addMesh("assets/models/tempRock.obj");
		this->rockFenceMeshId = resourceMan->addMesh("assets/models/rockFence.obj");
		this->tileFloorMeshId = resourceMan->addMesh("assets/models/Tiles/Floor.obj");
	}
}

void RoomHandler::roomCompleted()
{
	Room& curRoom = this->rooms[this->activeIndex];
	curRoom.finished = true;
	if (curRoom.type != RoomData::EXIT_ROOM)
	{
		glm::vec3 pos = scene->getComponent<Transform>(this->rooms[this->activeIndex].rock).position;
		this->scene->setScriptComponent(this->rooms[this->activeIndex].rock, "scripts/moveRock.lua");
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setScriptComponent(curRoom.doors[i], "scripts/opendoor.lua");
			this->scene->removeComponent<Collider>(curRoom.doors[i]);
		}
	}

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

void RoomHandler::generate(uint32_t seed)
{
	this->random = new VRandom;
	this->random->srand(seed);

	RoomLayout roomLayout(*this->random);
	RoomGenerator roomGen(*this->random);

	this->reset();

	roomLayout.generate();
	const int numTotRooms = roomLayout.getNumRooms();
	const int numMainRooms = roomLayout.getNumMainRooms();
	this->rooms.resize(numTotRooms);
	this->roomExitPoints.resize(numTotRooms);

	bool hasDoor[4]{};
	for (int i = 0; i < numTotRooms; i++)
	{
		const RoomData& roomData = roomLayout.getRoom(i);
		Room& curRoom = this->rooms[i];

		//curRoom.position = roomData.position;
		curRoom.type = roomData.type;

		// Save connecting indices
		curRoom.connectingIndex[LEFT_P]  = roomData.left;
		curRoom.connectingIndex[RIGHT_P] = roomData.right;
		curRoom.connectingIndex[UPPER_P] = roomData.up;
		curRoom.connectingIndex[LOWER_P] = roomData.down;

		hasDoor[LEFT_P]  = roomData.left != -1;
		hasDoor[RIGHT_P] = roomData.right != -1;
		hasDoor[UPPER_P] = roomData.up != -1;
		hasDoor[LOWER_P] = roomData.down != -1;

		roomGen.generate(hasDoor);
		const glm::ivec2* exits = roomGen.getExits();

		curRoom.extents[LEFT_P]  = std::abs((float)exits[LEFT_P].x)  * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[RIGHT_P] = std::abs((float)exits[RIGHT_P].x) * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[UPPER_P] = std::abs((float)exits[UPPER_P].y) * TILE_WIDTH - TILE_WIDTH * 0.5f;
		curRoom.extents[LOWER_P] = std::abs((float)exits[LOWER_P].y) * TILE_WIDTH - TILE_WIDTH * 0.5f;

		const glm::ivec2& size = roomGen.getSize();
		curRoom.box.extents.x = size.x * TILE_WIDTH;
		curRoom.box.extents.y = TILE_WIDTH * 1.f;
		curRoom.box.extents.z = size.y * TILE_WIDTH;

		printf("(%d, %d, %d)\n", (int)curRoom.box.extents.x, (int)curRoom.box.extents.y, (int)curRoom.box.extents.z);
		curRoom.box.offset.y = TILE_WIDTH * 1.f;
		curRoom.box = Collider::createBox(curRoom.box.extents, curRoom.box.offset, true);


		this->createDoors(i, exits);

		for (int j = 0; j < 4; j++)
		{
			if (curRoom.connectingIndex[j] != -1)
			{
				this->roomExitPoints[i].positions[j] = glm::vec3(exits[j].x, 0.f, exits[j].y) * TILE_WIDTH;
			}
		}

		const uint32_t numMain = roomGen.getNumMainTiles();
		for (uint32_t j = 0; j < numMain; j++)
		{
			const glm::vec2& tilePos = roomGen.getMainTile(j).position;
			if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
			{
				curRoom.objects.emplace_back(this->createFloorDecoEntity(tilePos, true));
			}

			curRoom.mainTiles.emplace_back(tilePos.x, 0.f, tilePos.y);
			curRoom.mainTiles.back() *= TILE_WIDTH;
		}

		const uint32_t numBig = roomGen.getNumBigTiles();
		for (uint32_t j = 0; j < numBig; j++)
		{
			this->createObjectEntities(roomGen.getBigTile(j), curRoom);
		}

		const uint32_t numExit = roomGen.getNumExitTiles();
		for (uint32_t j = 0; j < numExit; j++)
		{
			if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
			{
				curRoom.objects.emplace_back(this->createFloorDecoEntity(roomGen.getExitTile(j).position, true));
			}
		}

		const uint32_t numBorders = roomGen.getNumBorders();
		for (uint32_t j = 0; j < numBorders; j++)
		{
			curRoom.objects.emplace_back(this->createBorderEntity(roomGen.getBorder(j).position, true));
		}

		const uint32_t numInnerBorders = roomGen.getNumInnerBorders();
		for (uint32_t j = 0; j < numInnerBorders; j++)
		{
			Entity entity = this->createBorderEntity(roomGen.getInnerBorder(j).position, true);
			curRoom.objects.emplace_back(entity);
			//this->scene->setComponent<Collider>(entity, Collider::createBox(
			//	glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 3.f, TILE_WIDTH * 0.5f), glm::vec3(0.f, TILE_WIDTH * 3.f, 0.f)));

			if (this->useMeshes)
			{
				this->scene->getComponent<MeshComponent>(entity).meshID = this->innerBorderMesh;
			}
		}

		if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
		{
			Entity entity = this->scene->createEntity();
			this->scene->getComponent<Transform>(entity).position += glm::vec3(TILE_WIDTH * 0.5f, 0.f, TILE_WIDTH * 0.5f);
			curRoom.rockFence = entity;
			if (this->useMeshes)
			{
				this->scene->setComponent<MeshComponent>(entity, (int)this->rockFenceMeshId);
			}

			entity = this->scene->createEntity();
			Transform& rockTra = this->scene->getComponent<Transform>(entity);
			rockTra.position = glm::vec3(8.15f, -36.5f, -7.2f);
			rockTra.position += glm::vec3(TILE_WIDTH * 0.5f, 0.f, TILE_WIDTH * 0.5f);
			rockTra.rotation = glm::vec3(11.0f, 0.5f, 12.7f);
			curRoom.rock = entity;
			if (this->useMeshes)
			{
				this->scene->setComponent<MeshComponent>(entity, (int)rockMeshId);
			}

		}
		roomGen.clear();
	}

	for (int i = 0; i < numMainRooms; i++)
	{
		if (this->rooms[i].connectingIndex[LOWER_P] != -1)
		{
			Room& other = this->rooms[rooms[i].connectingIndex[LOWER_P]];
			const float offset = other.position.z + other.extents[UPPER_P] + this->rooms[i].extents[LOWER_P]
				+ TILES_BETWEEN_ROOMS * TILE_WIDTH;

			this->moveRoom(i, glm::vec3(0.f, 0.f, offset));
		}
		this->placeBranch(i, this->rooms[i].connectingIndex[LEFT_P], this->rooms[i].connectingIndex[RIGHT_P]);
	}
	this->setConnections(roomLayout.getNumMainRooms(), roomLayout.getConnections());
	this->generatePathways();
	this->createColliders();

	for (Entity entity : this->pathEntities)
	{
		this->scene->getComponent<Transform>(entity).position += glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f);
	}

	// Don't remove until server and clients always generate identical rooms
	//printf("Num Rooms: %zd | numPathsEntities: %zd\n", rooms.size(), pathEntities.size());
	for (int i = 0; i < numTotRooms; i++)
	{
		this->moveRoom(i, glm::vec3(TILE_WIDTH * -0.5f, 0.f, TILE_WIDTH * -0.5f));
		this->rooms[i].mainTiles.shrink_to_fit();
		this->rooms[i].objects.shrink_to_fit();

		this->rooms[i].boxEntityTemp = scene->createEntity();
		scene->getComponent<Transform>(this->rooms[i].boxEntityTemp).position = this->rooms[i].position;
		scene->setComponent<Collider>(this->rooms[i].boxEntityTemp, this->rooms[i].box);

		// Don't remove until server and clients always generate identical rooms
		//printf("Room: %d\n", i);
		//printf("Num mainTiles: %zd | numObjects: %zd\n", rooms[i].mainTiles.size(), rooms[i].objects.size());
		//printf("Connecting idx: 0: %d, 1: %d, 2: %d, 3: %d\n", rooms[i].connectingIndex[0], rooms[i].connectingIndex[1], rooms[i].connectingIndex[2], rooms[i].connectingIndex[3]);
		//printf("Type: %d | pos: (%d, %d, %d)\n----------\n", (int)rooms[i].type, (int)rooms[i].position.x, (int)rooms[i].position.y, (int)rooms[i].position.z);

	}

	roomLayout.clear();
	this->exitPairs.clear();
	this->roomExitPoints.clear();
	this->verticalConnection.clear();

	this->exitPairs.shrink_to_fit();
	this->roomExitPoints.shrink_to_fit();
	this->verticalConnection.shrink_to_fit();

	delete this->random;
	return;

#ifdef _CONSOLE
	if (this->showAllRooms)
	{
		for (int i = 0; i < numTotRooms; i++)
		{
			this->activateRoom(i);
		}
		this->showPaths(true);
	}
	else
	{
		for (int i = 0; i < numTotRooms; i++)
		{
			if (i != this->activeIndex)
			{
				this->deactivateRoom(i);
			}
		}
		this->showPaths(true);
	}
#else
	for (int i = 0; i < numTotRooms; i++)
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

void RoomHandler::moveRoom(int roomIndex, const glm::vec3& offset)
{
	Room& curRoom = this->rooms[roomIndex];

	curRoom.position += offset;
	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->getComponent<Transform>(curRoom.rock).position += offset;
		this->scene->getComponent<Transform>(curRoom.rockFence).position += offset;
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->roomExitPoints[roomIndex].positions[i] += offset;

			this->scene->getComponent<Transform>(curRoom.doors[i]).position += offset;
		}
	}

	for (glm::vec3& pos : curRoom.mainTiles)
	{
		pos += offset;
	}
	for (Entity entity : curRoom.objects)
	{
		this->scene->getComponent<Transform>(entity).position += offset;
	}
}

void RoomHandler::placeBranch(int index, int left, int right)
{
	Room& curRoom = this->rooms[index];
	glm::vec3 offset = curRoom.position;

	if (left != -1)
	{
		Room& leftRoom = this->rooms[left];
		offset.x += curRoom.extents[LEFT_P] + leftRoom.extents[RIGHT_P] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		this->moveRoom(left, offset);

		if (leftRoom.connectingIndex[LEFT_P] != -1)
		{
			this->placeBranch(left, leftRoom.connectingIndex[LEFT_P], -1);
		}
	}

	offset = curRoom.position;
	if (right != -1)
	{
		Room& rightRoom = this->rooms[right];
		offset.x -= curRoom.extents[RIGHT_P] + rightRoom.extents[LEFT_P] + TILES_BETWEEN_ROOMS * TILE_WIDTH;
		this->moveRoom(right, offset);

		if (rightRoom.connectingIndex[RIGHT_P] != -1)
		{
			this->placeBranch(right, -1, rightRoom.connectingIndex[RIGHT_P]);
		}
	}
}

Entity RoomHandler::createFloorDecoEntity(const glm::vec2& pos, bool scalePos)
{
	Entity entity = scene->createEntity();
	Transform& transform = scene->getComponent<Transform>(entity);
	transform.position.x = pos.x * (scalePos ? TILE_WIDTH : 1.f) + float((int)this->random->rand() % 12 - 6);
	transform.position.z = pos.y * (scalePos ? TILE_WIDTH : 1.f) + float((int)this->random->rand() % 12 - 6);
	transform.rotation.y = float(this->random->rand() % 360);
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->oneXOneMeshIds[this->random->rand() % NUM_ONE_X_ONE]);
	}
	return entity;
}

const std::vector<glm::vec3>& RoomHandler::getFreeTiles()
{
	return this->rooms[this->activeIndex].mainTiles;
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

void RoomHandler::createDoors(int roomIndex, const glm::ivec2* doorTilePos)
{
	Room& curRoom = this->rooms[roomIndex];

	const float rots[4] = { -90.f, 90.f, 180.f, 0.f };

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
		if (curRoom.connectingIndex[i] != -1)
		{
			curRoom.doors[i] = this->createDoorEntity(rots[i]);

			Transform& tra = this->scene->getComponent<Transform>(curRoom.doors[i]);
			tra.position.x = ((float)doorTilePos[i].x + OFFSETS[i].x) * TILE_WIDTH;
			tra.position.z = ((float)doorTilePos[i].y + OFFSETS[i].y) * TILE_WIDTH;
		}
	}
}

void RoomHandler::setConnections(int numMainRooms, const std::vector<glm::ivec2>& connections)
{
	this->exitPairs.resize(connections.size());
	this->verticalConnection.resize(connections.size());

	for (size_t i = 0; i < connections.size(); i++)
	{
		const glm::ivec2& curCon = connections[i];
		std::pair<glm::vec3, glm::vec3>& pair = this->exitPairs[i];

		if (curCon.x < numMainRooms && curCon.y < numMainRooms)
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

void RoomHandler::generatePathways()
{
	glm::vec3 delta{};
	glm::vec3 step{};
	glm::vec3 curPos{};
	Entity entity = -1;

	std::vector<glm::vec3> pathPositions;
	pathPositions.reserve(size_t(TILES_BETWEEN_ROOMS * 2u));

	// Will depend on where RoomGenerator places doors, but the while-loop has never gotten stuck yet tho
	const uint32_t maxIterations = TILES_BETWEEN_ROOMS * 4u;
	uint32_t numIterations = 0;

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

		// Calcuate the path and save the positions
		numIterations = 0;
		while (numIterations++ < maxIterations)
		{
			pathPositions.emplace_back(curPos);

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

		// Go through the path and generate a border around it
		const size_t startIdx = this->pathEntities.size();
		for (int l = 1; l <= 3; l++) // Thiccness off border around paths
		{
			this->surroundPaths(startIdx, pathPositions, p0, p1, (float)l, this->verticalConnection[i], l == 1);
		}

		for (const glm::vec3& pos : pathPositions)
		{
			if (this->random->rand() % 100 < DECO_ENTITY_CHANCE)
			{
				this->pathEntities.emplace_back(createFloorDecoEntity(glm::vec2(pos.x, pos.z), false));
			}
		}

		pathPositions.clear();
	}

	this->pathEntities.shrink_to_fit();
}

void RoomHandler::surroundPaths(size_t startIdx, const std::vector<glm::vec3>& pathPos, glm::vec3 p0, glm::vec3 p1, float distFactor, bool vertical, bool colliders)
{
	const glm::vec3 OFFSETS[] =
	{
		{TILE_WIDTH, 0, 0},
		{-TILE_WIDTH, 0, 0},
		{0, 0, TILE_WIDTH},
		{0, 0, -TILE_WIDTH},
	};
	const int NUM_OFFSETS = sizeof(OFFSETS) / sizeof(glm::vec3);

	bool canPlace = true;

	const glm::vec3& upperP = p0.z > p1.z ? p0 : p1;
	const glm::vec3& lowerP = upperP == p0 ? p1 : p0;
	const glm::vec3& rightP = p0.x > p1.x ? p0 : p1;
	const glm::vec3& leftP = rightP == p0 ? p1 : p0;

	for (size_t i = 0; i < pathPos.size(); i++)
	{
		const glm::vec3& pos = pathPos[i];

		for (int k = 0; k < NUM_OFFSETS; k++)
		{
			const glm::vec3 offsetPos = pos + OFFSETS[k] * distFactor;
			canPlace = true;

			// Don't place a border inside the room
			if (vertical)
			{
				canPlace = !(offsetPos.z > upperP.z || offsetPos.z < lowerP.z);
			}
			else
			{
				canPlace = !(offsetPos.x > rightP.x || offsetPos.x < leftP.x);
			}

			// Search through the path to see if offsetPos is on a tile
			for (size_t j = 0; j < pathPos.size() && canPlace; j++)
			{
				if (j != i)
				{
					glm::vec3 mPosToOffset = offsetPos - pathPos[j];
					mPosToOffset.y = 0.f;

					if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
					{
						canPlace = false;
					}
				}
			}

			// Search through the borders so no duplicates are made
			for (size_t j = startIdx; j < this->pathEntities.size() && canPlace; j++)
			{
				glm::vec3 mPosToOffset = offsetPos - this->scene->getComponent<Transform>(this->pathEntities[j]).position;
				mPosToOffset.y = 0.f;

				if (glm::dot(mPosToOffset, mPosToOffset) < (TILE_WIDTH * TILE_WIDTH))
				{
					canPlace = false;
				}
			}

			if (canPlace)
			{
				Entity entity = createBorderEntity({ offsetPos.x, offsetPos.z }, false);
				this->pathEntities.emplace_back(entity);

				if (colliders) // switch mesh and create collider for inner-layer borders
				{
					//this->scene->setComponent<Collider>(entity, Collider::createBox(
					//	glm::vec3(TILE_WIDTH * 0.5f, TILE_WIDTH * 3.f, TILE_WIDTH * 0.5f), glm::vec3(0.f, TILE_WIDTH * 3.f, 0.f)));

					if (this->useMeshes)
					{
						this->scene->getComponent<MeshComponent>(entity).meshID = this->innerBorderMesh;
					}
				}
			}
		}
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
		for (int i = 0; i < 4; i++)
		{
			if (room.doors[i] != -1)
			{
				/*room.doorTriggers[i] = this->scene->createEntity();

				Transform& doorTra = this->scene->getComponent<Transform>(room.doors[i]);
				Transform& triggerTra = this->scene->getComponent<Transform>(room.doorTriggers[i]);

				triggerTra.position = doorTra.position + offsets[i];
				triggerTra.rotation = doorTra.rotation;

				this->scene->setComponent<Collider>(room.doorTriggers[i],
					Collider::createBox(doorTrigCol, glm::vec3(0.f), true));*/
			}
		}

		if (room.rockFence != -1)
		{
			this->scene->setComponent<Collider>(room.rockFence,
				Collider::createBox(glm::vec3(TILE_WIDTH), glm::vec3(0.f, TILE_WIDTH, 0.f)));
		}
	}

	float minX = 10000000.f;
	float maxX = -10000000.f;
	float minZ = 10000000.f;
	float maxZ = -10000000.f;
	float extents[4]{};

	for (Room& room : this->rooms)
	{
		if (room.position.x < minX) { minX = room.position.x; extents[LEFT_P] = room.extents[LEFT_P]; }
		if (room.position.x > maxX) { maxX = room.position.x; extents[RIGHT_P] = room.extents[RIGHT_P]; }
		if (room.position.z < minZ) { minZ = room.position.z; extents[UPPER_P] = room.extents[UPPER_P]; }
		if (room.position.z > maxZ) { maxZ = room.position.z; extents[LOWER_P] = room.extents[LOWER_P]; }
	}

	const glm::vec3 floorPos((minX + maxX) * 0.5f, 0.f, (minZ + maxZ) * 0.5f);
	const glm::vec3 floorDimensions((maxX - minX) * 0.5f + extents[LEFT_P] + extents[RIGHT_P], 4.f, (maxZ - minZ) * 0.5f + +extents[UPPER_P] + extents[LOWER_P]);

	this->floor = this->scene->createEntity();
	this->scene->setComponent<Collider>(this->floor, Collider::createBox(floorDimensions, glm::vec3(0.f, -4.f, 0.f)));
	this->scene->getComponent<Transform>(this->floor).position = floorPos;
	this->scene->getComponent<Transform>(this->floor).scale = glm::vec3(floorDimensions.x * 2.f, 1.f, floorDimensions.z * 2.f);
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(this->floor, (int)this->tileFloorMeshId);
	}
}

Entity RoomHandler::createDoorEntity(float yRotation)
{
	Entity entity = scene->createEntity();
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.rotation.y = yRotation;

	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->doorMeshID);
	}

	return entity;
}

Entity RoomHandler::createBorderEntity(const glm::vec2& position, bool scalePos)
{
	Entity entity = this->scene->createEntity();
	Transform& transform = this->scene->getComponent<Transform>(entity);
	transform.position.x = position.x * (scalePos ? TILE_WIDTH : 1.f);
	transform.position.z = position.y * (scalePos ? TILE_WIDTH : 1.f);
	transform.rotation.y = 90.f * float(this->random->rand() % 4);
	
	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(entity, (int)this->borderMeshIds[this->random->rand() % NUM_BORDER]);
	}

	return entity;
}

void RoomHandler::createObjectEntities(const Tile& tile, Room& room)
{
	Entity mainEntity = this->scene->createEntity();
	room.objects.emplace_back(mainEntity);

	Transform& transform = this->scene->getComponent<Transform>(mainEntity);
	transform.position = glm::vec3(tile.position.x, 0.f, tile.position.y);
	transform.position *= TILE_WIDTH;
	transform.position.x += float((int)this->random->rand() % 10 - 5);
	transform.position.z += float((int)this->random->rand() % 10 - 5);

	if (tile.type == Tile::TwoXOne || tile.type == Tile::OneXTwo)
	{
		if (tile.type == Tile::OneXTwo)
		{
			transform.rotation.y = 90.f;
		}

		transform.rotation.y += this->random->rand() % 2 ? 180.f : 0.f;
		transform.rotation.y += float((int)this->random->rand() % 10 - 5);
	}
	else if (tile.type == Tile::TwoXTwo)
	{
		transform.rotation.y = float((int)this->random->rand() % 360);
	}

	std::pair<uint32_t, uint32_t> pair(~0u, ~0u);

	if		(tile.type == Tile::TwoXTwo) { pair = this->twoXTwoMeshIds[this->random->rand() % NUM_TWO_X_TWO]; }
	else if (tile.type == Tile::TwoXOne) { pair = this->oneXTwoMeshIds[this->random->rand() % NUM_ONE_X_TWO]; }
	else if (tile.type == Tile::OneXTwo) { pair = this->oneXTwoMeshIds[this->random->rand() % NUM_ONE_X_TWO]; }

	if (this->useMeshes)
	{
		this->scene->setComponent<MeshComponent>(mainEntity, (int)pair.first);
	}
	
	if (pair.second != ~0u)
	{
		std::vector<ColliderDataRes> colliders = resourceMan->getCollisionShapeFromMesh(pair.second);
		Entity collEntity;
		for (size_t i = 0; i < colliders.size(); i++)
		{
			collEntity = this->scene->createEntity();
			this->scene->setComponent<Collider>(collEntity, colliders[i].col);
			Transform& collTra = this->scene->getComponent<Transform>(collEntity);
			collTra.position = transform.position + colliders[i].position;
			collTra.rotation = transform.rotation + colliders[i].rotation;

			room.objects.emplace_back(collEntity);
		}
	}
}

bool RoomHandler::checkRoom(int index, Entity otherEntity)
{
	return false;

	bool newUnfinshedRoom = false;
	Room& curRoom = this->rooms[index];

	for (int i = 0; i < 4; i++)
	{
		Entity entity;// = curRoom.doorTriggers[i];

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
		for (Entity tile : this->pathEntities)
		{
			this->scene->setActive(tile);
		}
	}
	else
	{
		for (Entity tile : this->pathEntities)
		{
			this->scene->setInactive(tile);
		}
	}
}

void RoomHandler::reset()
{
	for (Room& room : this->rooms)
	{
		for (const Entity& entity : room.objects)
		{
			this->scene->removeEntity(entity);
		}

		room.mainTiles.clear();
		room.objects.clear();

		for (int i = 0; i < 4; i++)
		{
			if (room.doors[i] != -1)
			{
				this->scene->removeEntity(room.doors[i]);
				//this->scene->removeEntity(room.doorTriggers[i]);
			}
			room.doors[i] = -1;
			//room.doorTriggers[i] = -1;
			room.connectingIndex[i] = -1;
			room.extents[i] = 0.f;
		}

		room.position = glm::vec3(0.f);
		room.finished = false;
		this->scene->removeEntity(room.rock);
		this->scene->removeEntity(room.rockFence);
		this->scene->removeEntity(room.boxEntityTemp);
	}
	this->rooms.clear();

	for (const Entity& entity : this->pathEntities)
	{
		this->scene->removeEntity(entity);
	}
	this->pathEntities.clear();

	this->scene->removeEntity(this->floor);
	this->floor = -1;

	this->exitPairs.clear();
	this->roomExitPoints.clear();

	this->activeIndex = 0;
	this->nextIndex = -1;
	this->curDoor = -1;
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
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setActive(entity);
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setActive(curRoom.doors[i]);
			//this->scene->setActive(curRoom.doorTriggers[i]);
			this->scene->getComponent<Transform>(curRoom.doors[i]).position.y = -25;
		}
	}

	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->setActive(curRoom.rock);
		this->scene->setActive(curRoom.rockFence);
	}
}

void RoomHandler::deactivateRoom(int index)
{
	if (index < 0 || index >= (int)this->rooms.size())
	{
		return;
	}

	Room& curRoom = this->rooms[index];
	for (const Entity& entity : curRoom.objects)
	{
		this->scene->setInactive(entity);
	}

	for (int i = 0; i < 4; i++)
	{
		if (curRoom.doors[i] != -1)
		{
			this->scene->setInactive(curRoom.doors[i]);
			//this->scene->setInactive(curRoom.doorTriggers[i]);
		}
	}
	if (curRoom.type != RoomData::START_ROOM && curRoom.type != RoomData::EXIT_ROOM)
	{
		this->scene->setInactive(curRoom.rock);
		this->scene->setInactive(curRoom.rockFence);
	}
}

#ifdef _CONSOLE
void RoomHandler::imgui(DebugRenderer* dr)
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
		if (ImGui::Button("Reload"))
		{
			this->generate(rand());
		}

		static bool drawExtents = false;
		ImGui::Checkbox("extents", &drawExtents);

		if (drawExtents)
		{
			glm::vec3 endPos;
			for (auto& room : rooms)
			{
				endPos = room.position + glm::vec3(room.extents[0], 0.f, 0.f);
				dr->renderLine(room.position, endPos, glm::vec3(1.f, 0.f, 0.f));

				endPos = room.position + glm::vec3(-room.extents[1], 0.f, 0.f);
				dr->renderLine(room.position, endPos, glm::vec3(1.f, 1.f, 0.f));

				endPos = room.position + glm::vec3(0.f, 0.f, room.extents[2]);
				dr->renderLine(room.position, endPos, glm::vec3(0.f, 0.f, 1.f));

				endPos = room.position + glm::vec3(0.f, 0.f, -room.extents[3]);
				dr->renderLine(room.position, endPos, glm::vec3(0.f, 1.f, 1.f));
			}
		}

	}
	ImGui::End();
#endif
}

#endif // _CONSOLE

