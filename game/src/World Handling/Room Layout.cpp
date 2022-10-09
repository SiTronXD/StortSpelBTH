#include "Room Layout.h"
#include "vengine/application/Scene.hpp"
#include "vengine/application/Input.hpp"
#include "vengine/application/Time.hpp"
#include "vengine/components/MeshComponent.hpp"
#include "Randomizer.hpp"

#include <ctime>

RoomLayout::RoomLayout()
	:boss(-1), doors{-1, -1, -1, -1}, foundBoss(false), roomID(-1), roomDims(100.f)
{
	
}

RoomLayout::~RoomLayout()
{
}

void RoomLayout::init(Scene* scene, const glm::vec3& roomDims)
{
	this->scene = scene;
	this->roomDims = roomDims;
}

void RoomLayout::generate()
{
	foundBoss = false;
	roomID = 0;

	boss = scene->createEntity();
	//this->setComponent<MeshComponent>(boss);
	scene->getComponent<Transform>(boss).position = glm::vec3(-1000.0f, -1000.0f, -1000.0f);

	for (int i = 0; i < 4; i++)
	{
		doors[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(doors[i]);
		scene->getComponent<Transform>(doors[i]).scale.y = 50.f;
	}
	initRooms();

	//std::cout << "Slow: WASD" << std::endl << "Fast: HBNM" << std::endl;
}

void RoomLayout::clear()
{
	for (int i = 0; i < 4; i++)
	{
		scene->removeEntity(doors[i]);
		doors[i] = -1;
	}
	scene->removeEntity(boss);
	boss = -1;

	for (int entity : rooms)
	{
		scene->removeEntity(entity);
	}

	rooms.clear();
	foundBoss = false;
}

std::string RoomLayout::typeToString(Room::ROOM_TYPE type)
{
	return std::string();
}

void RoomLayout::initRooms()
{
	std::srand((unsigned)time(0));

	int numRooms = setUpRooms();
	int numBranches = rand() % numRooms + 1;

	printf("Main rooms: %d, branches: %d\n", numRooms, numBranches);

	for (int i = 0; i < numBranches; i++)
	{
		if (!setRandomBranch(numRooms)) {
			std::cout << "Room: Could not create branch.\n";
		}
	}
	if (!setBoss(numRooms)) {
		std::cout << "Room: Could not create boss room.\n";
	}
	if (!setExit()) {
		std::cout << "Room: Could not create exit.\n";
	}
	if (!setShortcut(numBranches, numRooms)) {
		std::cout << "Room: Could not create shortcut.\n";
	}
	//placeDoors(roomID);
}

int RoomLayout::setUpRooms()
{
	const float MIN_WIDTH = 50.0f;
	const float MAX_WIDTH = 200.0f;
	const float MIN_HEIGHT = 50.0f;
	const float MAX_HEIGHT = 200.0f;
	const float MIN_DEPTH = 50.0f;
	const float MAX_DEPTH = 200.0f;

	const float MIN_X_POS_SPREAD = -50.0f;
	const float MAX_X_POS_SPREAD = 50.0f;
	const float MIN_Y_POS_SPREAD = 0.0f;
	const float MAX_Y_POS_SPREAD = 0.0f;
	const float MIN_Z_POS_SPREAD = 0.0f;
	const float MAX_Z_POS_SPREAD = 20.0f;

	// 3 - 6
	int numRooms = rand() % 2 + 4;

	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numRooms; i++)
	{
		// Create entity (already has transform)
		rooms.push_back(scene->createEntity());
		scene->setComponent<Room>(rooms[i]);

		Room& curRoom = scene->getComponent<Room>(rooms[i]);
		Transform& curTransform = scene->getComponent<Transform>(rooms[i]);
		glm::vec3& curPos = curTransform.position;
		glm::vec3& dimensions = curRoom.dimensions;

		if (i > 0)
		{
			scene->getComponent<Room>(rooms[i - 1]).up = i;
			scene->getComponent<Room>(rooms[i]).down = i - 1;
		}

#if RANDOM_POSITION
		dimensions = getRandomVec3(MIN_WIDTH, MAX_WIDTH, MIN_HEIGHT, MAX_HEIGHT, MIN_DEPTH, MAX_DEPTH);
#else
		dimensions = roomDims;
		curPos.z = i * roomDims.z * 1.5f;
#endif


		//First room is alwas the start room
		if (i == 0)
		{
			curRoom.type = Room::ROOM_TYPE::START_ROOM;
			curPos = glm::vec3(0.0f, 0.0f, 0.0f);

		}
		else
		{
			//one in five to become a hard room
			if (rand() % 5 == 0)
			{
				curRoom.type = Room::ROOM_TYPE::HARD_ROOM;
			}
			else
			{
				curRoom.type = Room::ROOM_TYPE::NORMAL_ROOM;
			}

#if RANDOM_POSITION
			offset.x = dimensions.x / 2.0f + scene->getComponent<Room>(rooms[i - 1]).dimensions.x / 2.0f;
			offset.y = 0.0f;// dimensions.y / 2.0f + scene->getComponent<Room>(rooms[i - 1]).dimensions.y / 2.0f;
			offset.z = dimensions.z / 2.0f + scene->getComponent<Room>(rooms[i - 1]).dimensions.z / 2.0f;

			float minX = curPos.x - offset.x + MIN_X_POS_SPREAD;
			float maxX = curPos.x + offset.x + MAX_X_POS_SPREAD;
			float minY = curPos.y + offset.y + MIN_Y_POS_SPREAD;
			float maxY = curPos.y + offset.y + MAX_Y_POS_SPREAD;
			float minZ = curPos.z + offset.z + MIN_Z_POS_SPREAD;
			float maxZ = curPos.z + offset.z + MAX_Z_POS_SPREAD;

			curPos = getRandomVec3(minX, maxX, minY, maxY, minZ, maxZ);		
#endif
		}
	}

	return numRooms;
}

bool RoomLayout::setRandomBranch(int numRooms)
{
	int branchSize = rand() % 2 + 1;

	bool foundSpot = false;
	int numMainRooms = numRooms;
	int spot = rand() % (numMainRooms - 1);
	int numTest = 0;
	if (scene->getComponent<Room>(rooms[spot]).left != -1 && scene->getComponent<Room>(rooms[spot]).right != -1)
	{
		//Keep looking for a spot to place branch
		while (!foundSpot)
		{
			if (scene->getComponent<Room>(rooms[spot]).left == -1 || scene->getComponent<Room>(rooms[spot]).right == -1)
			{
				foundSpot = true;
				break;
			}
			if (++spot >= numMainRooms)
			{
				spot = 0;
			}
			if (++numTest > numRooms) {
				return false;
			}
		}
	}
	//Found spot
	if (scene->getComponent<Room>(rooms[spot]).branch) {
		int test = 0;
	}
	if (scene->getComponent<Room>(rooms[spot]).left == -1 && scene->getComponent<Room>(rooms[spot]).right == -1)
	{
		if (rand() % 2 == 0)
		{
			setBranch(spot, true, branchSize);
		}
		else
		{
			setBranch(spot, false, branchSize);
		}
	}
	else if (scene->getComponent<Room>(rooms[spot]).left == -1)
	{
		setBranch(spot, true, branchSize);
	}
	else
	{
		setBranch(spot, false, branchSize);
	}

	return true;
}

void RoomLayout::setBranch(int index, bool left, int size)
{
	const float MIN_WIDTH = 50.0f;
	const float MAX_WIDTH = 200.0f;
	const float MIN_HEIGHT = 50.0f;
	const float MAX_HEIGHT = 200.0f;
	const float MIN_DEPTH = 50.0f;
	const float MAX_DEPTH = 200.0f;


	const float MIN_X_POS_SPREAD = 50.0f;
	const float MAX_X_POS_SPREAD = 20.0f;
	const float MIN_Y_POS_SPREAD = 0.0f;
	const float MAX_Y_POS_SPREAD = 0.0f;
	const float MIN_Z_POS_SPREAD = -10.0f;
	const float MAX_Z_POS_SPREAD = 10.0f;

	Room::ROOM_TYPE roomType = Room::ROOM_TYPE::NORMAL_ROOM;

	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 position = scene->getComponent<Transform>(rooms[index]).position;

#if RANDOM_POSITION
	glm::vec3 dimensions = getRandomVec3(MIN_WIDTH, MAX_WIDTH, MIN_HEIGHT, MAX_HEIGHT, MIN_DEPTH, MAX_DEPTH);
#endif

	if (rand() % 5 == 0)
	{
		roomType = Room::ROOM_TYPE::HARD_ROOM;
	}
	else
	{
		roomType = Room::ROOM_TYPE::NORMAL_ROOM;
	}

	if (left)
	{
		for (int i = 0; i < size; i++)
		{
#if RANDOM_POSITION
			offset.x = dimensions.x / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.x / 2.0f;
			offset.y = 0.0f;// dimensions.y / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.y / 2.0f;
			offset.z = dimensions.z / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.z / 2.0f;

			float minX = position.x - offset.x - MIN_X_POS_SPREAD;
			float maxX = position.x + offset.x - MAX_X_POS_SPREAD;
			float minY = position.y + offset.y - MIN_Y_POS_SPREAD;
			float maxY = position.y + offset.y + MAX_Y_POS_SPREAD;
			float minZ = position.z + offset.z - MIN_Z_POS_SPREAD;
			float maxZ = position.z + offset.z + MAX_Z_POS_SPREAD;

			position = getRandomVec3(minX, maxX, minY, maxY, minZ, maxZ);
#else
			position.x += roomDims.x * 1.5f;
#endif

			rooms.push_back(scene->createEntity());
			scene->setComponent<Room>(rooms[rooms.size() - 1]);
			Room& roomRef = scene->getComponent<Room>(rooms[rooms.size() - 1]);
			glm::vec3& posRef = scene->getComponent<Transform>(rooms[rooms.size() - 1]).position;
			roomRef.branch = true;
			if (i == size - 1)
			{
				roomRef.branchEnd = true;
			}
			roomRef.type = roomType;
			posRef = position;

#if RANDOM_POSITION
			roomRef.dimensions = dimensions;
#else
			roomRef.dimensions = roomDims;
#endif

			int curRoomLeft, curRoomIndex;

			if (i == 0)
			{
				curRoomIndex = index;
				curRoomLeft = (int)rooms.size() - 1;
				scene->getComponent<Room>(rooms[curRoomIndex]).left = curRoomLeft;
				scene->getComponent<Room>(rooms[curRoomLeft]).right = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				curRoomLeft = (int)rooms.size() - 1;
				scene->getComponent<Room>(rooms[curRoomIndex]).left = curRoomLeft;
				scene->getComponent<Room>(rooms[curRoomLeft]).right = curRoomIndex;
			}

		}
	}
	else
	{

		for (int i = 0; i < size; i++)
		{
#if RANDOM_POSITION
			offset.x = dimensions.x / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.x / 2.0f;
			offset.y = 0.0f;// dimensions.y / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.y / 2.0f;
			offset.z = dimensions.z / 2.0f + scene->getComponent<Room>(rooms[index]).dimensions.z / 2.0f;

			float minX = position.x - offset.x + MIN_X_POS_SPREAD;
			float maxX = position.x + offset.x + MAX_X_POS_SPREAD;
			float minY = position.y + offset.y - MIN_Y_POS_SPREAD;
			float maxY = position.y + offset.y + MAX_Y_POS_SPREAD;
			float minZ = position.z + offset.z - MIN_Z_POS_SPREAD;
			float maxZ = position.z + offset.z + MAX_Z_POS_SPREAD;

			position = getRandomVec3(minX, maxX, minY, maxY, minZ, maxZ);
#else
			position.x -= roomDims.x * 1.5f;
#endif

			rooms.push_back(scene->createEntity());
			scene->setComponent<Room>(rooms[rooms.size() - 1]);
			Room& roomRef = scene->getComponent<Room>(rooms[rooms.size() - 1]);
			glm::vec3& posRef = scene->getComponent<Transform>(rooms[rooms.size() - 1]).position;
			roomRef.branch = true;
			if (i == size - 1) {
				roomRef.branchEnd = true;
			}
			roomRef.type = roomType;
			posRef = position;

#if RANDOM_POSITION
			roomRef.dimensions = dimensions;
#else
			roomRef.dimensions = roomDims;
#endif

			int curRoomRight, curRoomIndex;
			if (i == 0)
			{
				curRoomIndex = index;
				curRoomRight = (int)rooms.size() - 1;
				scene->getComponent<Room>(rooms[curRoomIndex]).right = curRoomRight;
				scene->getComponent<Room>(rooms[curRoomRight]).left = curRoomIndex;
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				curRoomRight = (int)rooms.size() - 1;
				scene->getComponent<Room>(rooms[curRoomIndex]).right = curRoomRight;
				scene->getComponent<Room>(rooms[curRoomRight]).left = curRoomIndex;
			}

		}
	}
}

bool RoomLayout::setBoss(int numRooms)
{
	int left = -1;
	int bossIndex = rand() % (numRooms / 2) + numRooms / 2;
	int numTest = 0;
	while (scene->getComponent<Room>(rooms[bossIndex]).left != -1 && scene->getComponent<Room>(rooms[bossIndex]).right != -1)
	{
		if (++bossIndex > numRooms)
		{
			bossIndex = 1;
		}
		if (++numTest > numRooms) {
			return false;
		}
	}
	if (scene->getComponent<Room>(rooms[bossIndex]).left == -1 && scene->getComponent<Room>(rooms[bossIndex]).right == -1)
	{
		left = rand() % 2;
	}
	else if (scene->getComponent<Room>(rooms[bossIndex]).left == -1)
	{
		left = 1;
	}
	else
	{
		left = 0;
	}
#if BRANCH == 1
	setBranch(bossIndex, left, rand() % 3 + 1);
#endif
	scene->getComponent<Room>(rooms[rooms.size() - 1]).type = Room::ROOM_TYPE::BOSS_ROOM;

	return true;
}

bool RoomLayout::setExit()
{
	int exitIndex = rand() % (rooms.size() - 1) + 1;
	int numTests = 0;
	while (scene->getComponent<Room>(rooms[exitIndex]).type == Room::ROOM_TYPE::BOSS_ROOM || scene->getComponent<Room>(rooms[exitIndex]).type == Room::ROOM_TYPE::START_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (scene->getComponent<Room>(rooms[exitIndex]).left != -1 && scene->getComponent<Room>(rooms[scene->getComponent<Room>(rooms[exitIndex]).left]).type == Room::ROOM_TYPE::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (scene->getComponent<Room>(rooms[exitIndex]).right != -1 && scene->getComponent<Room>(rooms[scene->getComponent<Room>(rooms[exitIndex]).right]).type == Room::ROOM_TYPE::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	scene->getComponent<Room>(rooms[exitIndex]).type = Room::ROOM_TYPE::EXIT_ROOM;

	return true;
}

bool RoomLayout::setShortcut(int numBranches, int numRooms)
{
	int one = getEndWithLeftAvaliable();
	int two = getEndWithRightAvaliable();
	if (one == -1 || two == -1)
	{
		return false;
	}
	scene->getComponent<Room>(rooms[one]).shortcut = true;
	scene->getComponent<Room>(rooms[one]).branchEnd = false;
	scene->getComponent<Room>(rooms[one]).left = two;
	scene->getComponent<Room>(rooms[two]).shortcut = true;
	scene->getComponent<Room>(rooms[two]).branchEnd = false;
	scene->getComponent<Room>(rooms[two]).right = one;

	return true;
}

int RoomLayout::numEnds()
{
	int ret = 0;
	for (int i = 0; i < rooms.size(); i++)
	{
		if (scene->getComponent<Room>(rooms[i]).branchEnd)
		{
			ret++;
		}
	}
	return ret;
}

int RoomLayout::getEndWithRightAvaliable()
{
	int ret = -1;
	for (int i = 0; i < rooms.size(); i++)
	{
		Room& curRoom = scene->getComponent<Room>(rooms[i]);
		if (curRoom.branchEnd && curRoom.right == -1)
		{
			ret = i;
			break;
		}
	}
	return ret;
}

int RoomLayout::getEndWithLeftAvaliable()
{
	int ret = -1;
	for (int i = 0; i < rooms.size(); i++)
	{
		Room& curRoom = scene->getComponent<Room>(rooms[i]);
		if (curRoom.branchEnd && curRoom.left == -1)
		{
			ret = i;
			break;
		}
	}
	return ret;
}

void RoomLayout::traverseRoomsConsole()
{
	std::string input;

	int id = 0;
	Room* curRoom = &scene->getComponent<Room>(rooms[id]);
	bool exit = false;
	bool foundBoss = false;
	while (!exit)
	{
		system("cls");
		if (curRoom->branch)
		{
			std::cout << "Branch ";
			if (curRoom->branchEnd)
			{
				std::cout << "end." << std::endl;
			}
			else
			{
				std::cout << std::endl;
			}
		}
		std::cout << "Current Room: " << typeToString(curRoom->type) << " ID: " << id << std::endl << "Choises: " << std::endl;
		if (curRoom->down != -1)
		{
			std::cout << "down\n";
		}
		if (curRoom->left != -1)
		{
			std::cout << "left\n";
		}
		if (curRoom->right != -1)
		{
			std::cout << "right\n";
		}
		if (curRoom->up != -1)
		{
			std::cout << "up\n";
		}
		std::cout << "exit\n";
		std::cout << "input room: ";
		std::cin >> input;

		if (input == "down" && curRoom->down != -1)
		{
			id = curRoom->down;
			curRoom = &scene->getComponent<Room>(rooms[curRoom->down]);
		}
		else if (input == "left" && curRoom->left != -1)
		{
			id = curRoom->left;
			curRoom = &scene->getComponent<Room>(rooms[curRoom->left]);
		}
		else if (input == "right" && curRoom->right != -1)
		{
			id = curRoom->right;
			curRoom = &scene->getComponent<Room>(rooms[curRoom->right]);
		}
		else if (input == "up" && curRoom->up != -1)
		{
			id = curRoom->up;
			curRoom = &scene->getComponent<Room>(rooms[curRoom->up]);
		}
		else if (input == "exit" || input == "end")
		{
			break;
		}

		if ((curRoom->type == Room::ROOM_TYPE::BOSS_ROOM) && !foundBoss)
		{
			foundBoss = true;
		}

		if ((curRoom->type == Room::ROOM_TYPE::EXIT_ROOM) && foundBoss)
		{
			system("cls");
			std::cout << "You found the exit!\nDo you want to exit? Y/n" << std::endl;
			std::cin >> input;
			if (input == "Y")
			{
				exit = true;
			}
		}
	}
}

bool RoomLayout::traverseRooms(int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta)
{
	bool ret = false;
	Room curRoom = scene->getComponent<Room>(rooms[roomID]);

	if (canGoForward() || (curRoom.up != -1 && Input::isKeyPressed(Keys::H))) {
		roomID = curRoom.up;
		placeDoors(roomID);
	}
	else if (canGoBack() || (curRoom.down != -1 && Input::isKeyPressed(Keys::N))) {
		roomID = curRoom.down;
		placeDoors(roomID);
	}
	else if (canGoLeft() || (curRoom.left != -1 && Input::isKeyPressed(Keys::M))) {
		roomID = curRoom.left;
		placeDoors(roomID);
	}
	else if (canGoRight() || (curRoom.right != -1 && Input::isKeyPressed(Keys::B))) {
		roomID = curRoom.right;
		placeDoors(roomID);
	}

	if (curRoom.type == Room::ROOM_TYPE::BOSS_ROOM && bossHealth > 0)
	{
		fightBoss(boss, bossHealth, roomID, foundBoss);
	}
	else
	{
		Transform& transform = scene->getComponent<Transform>(boss);
		transform.position = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
	}
	if (curRoom.type == Room::ROOM_TYPE::BOSS_ROOM && foundBoss == false) {
		foundBoss = true;
	}
	else if (curRoom.type == Room::ROOM_TYPE::EXIT_ROOM && foundBoss) {
		ret = true;
	}
	return ret;
}

void RoomLayout::placeDoors(int& roomID)
{
	Room& curRoom = scene->getComponent<Room>(rooms[roomID]);
	glm::vec3 curPos = scene->getComponent<Transform>(rooms[roomID]).position;
	glm::vec3& camPos = scene->getComponent<Transform>(scene->getMainCameraID()).position;

	printDoorOptions(roomID);

	camPos = curPos;

	glm::vec3 posLeft = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posRight = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posUp = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posDown = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	if (curRoom.left != -1) {
		posLeft = glm::vec3(curPos.x - curRoom.dimensions.x / 2, 0.0f, curPos.z);
	}
	if (curRoom.right != -1) {
		posRight = glm::vec3(curPos.x + curRoom.dimensions.x / 2, 0.0f, curPos.z);
	}
	if (curRoom.up != -1) {
		posUp = glm::vec3(curPos.x, 0.0f, curPos.z + curRoom.dimensions.z / 2);
	}
	if (curRoom.down != -1) {
		posDown = glm::vec3(curPos.x, 0.0f, curPos.z - curRoom.dimensions.z / 2);
	}
	scene->getComponent<Transform>(doors[0]).position = posLeft;
	scene->getComponent<Transform>(doors[1]).position = posRight;
	scene->getComponent<Transform>(doors[2]).position = posUp;
	scene->getComponent<Transform>(doors[3]).position = posDown;
}

bool RoomLayout::canGoForward()
{
	glm::vec3 doorPos = scene->getComponent<Transform>(doors[2]).position;
	glm::vec3 camPos = scene->getComponent<Transform>(scene->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool RoomLayout::canGoBack()
{
	glm::vec3 doorPos = scene->getComponent<Transform>(doors[3]).position;
	glm::vec3 camPos = scene->getComponent<Transform>(scene->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);

	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool RoomLayout::canGoLeft()
{
	glm::vec3 doorPos = scene->getComponent<Transform>(doors[0]).position;
	glm::vec3 camPos = scene->getComponent<Transform>(scene->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool RoomLayout::canGoRight()
{
	glm::vec3 doorPos = scene->getComponent<Transform>(doors[1]).position;
	glm::vec3 camPos = scene->getComponent<Transform>(scene->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

void RoomLayout::fightBoss(int& boss, int& bossHealth, int& roomID, bool& foundBoss)
{
	Transform& transform = scene->getComponent<Transform>(boss);
	transform.position = scene->getComponent<Transform>(rooms[roomID]).position + glm::vec3(cos(Time::getTimeSinceStart() * 100), sin(Time::getTimeSinceStart() * 100), 20.0f);
	transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
	transform.rotation = glm::vec3(transform.rotation.x + (Time::getDT() * 50), transform.rotation.y + (Time::getDT() * 50), transform.rotation.z + (Time::getDT() * 50));
	if (!foundBoss)
	{
		system("cls");
		std::cout << "Press H to fight the boss!\n Health: " << bossHealth << std::endl;
	}
	if (Input::isKeyPressed(Keys::H)) {
		system("cls");
		if (rand() % 10 == 0)
		{
			bossHealth -= 10;
			std::cout << bossHealth << std::endl << "-10 HUUUGE!" << std::endl;
		}
		else if (rand() % 5 == 0)
		{
			bossHealth -= 5;
			std::cout << bossHealth << std::endl << "-5 *CRITICAL HIT*" << std::endl;
		}
		else
		{
			bossHealth -= 1;
			std::cout << bossHealth << std::endl << "-1" << std::endl;

		}
		switch (rand() % 10)
		{
		case 0:
			std::cout << "ouch!";
			break;
		case 1:
			std::cout << "AJAJAJ!";
			break;
		case 2:
			std::cout << "NOO!";
			break;
		case 3:
			std::cout << "HOW DARE YOU?!";
			break;
		case 4:
			std::cout << "no dont touch me there!";
			break;
		case 5:
			std::cout << "I will kill you!";
			break;
		case 6:
			std::cout << "Fuck";
			break;
		case 7:
			std::cout << "Rat!";
			break;
		case 8:
			std::cout << "Ow!";
			break;
		case 9:
			std::cout << "shiii!";
			break;
		}
	}
	if (bossHealth <= 0)
	{
		placeDoors(roomID);
	}
}

void RoomLayout::printDoorOptions(int& roomID)
{
	Room& curRoom = scene->getComponent<Room>(rooms[roomID]);

	system("cls");
	if (curRoom.branch) {
		std::cout << "Branch ";
		if (curRoom.branchEnd) {
			std::cout << "end." << std::endl;
		}
		else {
			std::cout << std::endl;
		}
	}
	if (curRoom.shortcut) {
		std::cout << "Shortcut" << std::endl;
	}
	std::cout << "Current Room: " << typeToString(curRoom.type) << std::endl << " ID: " << roomID << std::endl << "Choises: " << std::endl;
	if (curRoom.up != -1) {
		std::cout << "up\n";
	}
	if (curRoom.down != -1) {
		std::cout << "down\n";
	}
	if (curRoom.right != -1) {
		std::cout << "left\n";
	}
	if (curRoom.left != -1) {
		std::cout << "right\n";
	}
}
