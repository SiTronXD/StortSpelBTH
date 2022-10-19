#include "Room Layout.h"
#include "vengine/application/Scene.hpp"
#include "vengine/application/Input.hpp"
#include "vengine/application/Time.hpp"
#include "vengine/components/MeshComponent.hpp"
#include "Randomizer.hpp"

#include <ctime>

RoomLayout::RoomLayout()
	:boss(-1), foundBoss(false), roomID(-1), distance(0.f)
{
	std::srand((unsigned)time(0));	
}

RoomLayout::~RoomLayout()
{
}

void RoomLayout::init(Scene* scene, float distance)
{
	this->scene = scene;
	this->distance = distance;
}

void RoomLayout::clear()
{
	boss = -1;
	foundBoss = false;

	rooms.clear();

	connections.clear();
}

void RoomLayout::generate()
{
	/*
		main rooms: 3 - 5
		branches: 1 - (numRooms + 1)
		branch size: 1-2
	*/

	numMainRooms = rand() % 3 + 3; 
	int numBranches = rand() % (numMainRooms + 1) + 1; 

	connections.reserve(size_t(numMainRooms + numBranches));

	printf("Main rooms: %d, branches: %d\n", numMainRooms, numBranches);

	setUpRooms(numMainRooms);
	for (int i = 0; i < numBranches; i++)
	{
		if (!setRandomBranch(numMainRooms)) {
			std::cout << "Room: Could not create branch.\n";
		}
	}

	/*if (!setBoss(numMainRooms)) {
		std::cout << "Room: Could not create boss room.\n";
	}
	if (!setExit()) {
		std::cout << "Room: Could not create exit.\n";
	}
	if (!setShortcut(numBranches, numRooms)) {
		std::cout << "Room: Could not create shortcut.\n";
	}*/
}

void RoomLayout::setUpRooms(int numRooms)
{
	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numRooms; i++)
	{
		// Create entity (already has transform)
		rooms.emplace_back();


		RoomData& curRoom = rooms[i];
		glm::vec3& curPosition = curRoom.position;

		curPosition.z = i * distance;

		//First room is alwas the start room
		if (i == 0)
		{
			curRoom.type = RoomData::Type::START_ROOM;
			curPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			if (i > 0)
			{
				rooms[i - 1].up = i;
				rooms[i].down = i - 1;

				connections.emplace_back(i, i - 1);
			}

			//one in five to become a hard room
			if (rand() % 5 == 0)
			{
				curRoom.type = RoomData::Type::HARD_ROOM;
			}
			else
			{
				curRoom.type = RoomData::Type::NORMAL_ROOM;
			}
		}
	}
}

bool RoomLayout::setRandomBranch(int numRooms)
{
	int branchSize = rand() % 2 + 1;

	bool foundSpot = false;
	int spot = rand() % (numRooms - 1);
	int numTest = 0;

	if (rooms[spot].left != -1 && rooms[spot].right != -1)
	{
		//Keep looking for a spot to place branch
		while (!foundSpot)
		{
			if (rooms[spot].left == -1 || rooms[spot].right == -1)
			{
				foundSpot = true;
				break;
			}
			if (++spot >= numRooms)
			{
				spot = 0;
			}
			if (++numTest > numRooms) {
				return false;
			}
		}
	}
	if (rooms[spot].left == -1 && rooms[spot].right == -1)
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
	else if (rooms[spot].left == -1)
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
	RoomData::Type roomType = RoomData::Type::NORMAL_ROOM;

	glm::vec3 position = rooms[index].position;

	if (rand() % 5 == 0)
	{
		roomType = RoomData::Type::HARD_ROOM;
	}
	else
	{
		roomType = RoomData::Type::NORMAL_ROOM;
	}

	if (left)
	{
		for (int i = 0; i < size; i++)
		{
			position.x += distance;

			rooms.emplace_back();

			RoomData& roomRef = rooms[rooms.size() - 1];
			glm::vec3& posRef = rooms[rooms.size() - 1].position;

			roomRef.branch = true;
			if (i == size - 1)
			{
				roomRef.branchEnd = true;
			}
			roomRef.type = roomType;
			posRef = position;


			int curRoomLeft, curRoomIndex;

			if (i == 0)
			{
				curRoomIndex = index;
				curRoomLeft = (int)rooms.size() - 1;
				rooms[curRoomIndex].left = curRoomLeft;
				rooms[curRoomLeft].right = curRoomIndex;
				connections.emplace_back(curRoomLeft, curRoomIndex);
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				curRoomLeft = (int)rooms.size() - 1;
				rooms[curRoomIndex].left = curRoomLeft;
				rooms[curRoomLeft].right = curRoomIndex;
				connections.emplace_back(curRoomLeft, curRoomIndex);
			}

		}
	}
	else
	{
		for (int i = 0; i < size; i++)
		{
			position.x -= distance;

			rooms.emplace_back();
			RoomData& roomRef = rooms[rooms.size() - 1];
			glm::vec3& posRef = rooms[rooms.size() - 1].position;

			roomRef.branch = true;
			if (i == size - 1) {
				roomRef.branchEnd = true;
			}
			roomRef.type = roomType;
			posRef = position;

			int curRoomRight, curRoomIndex;
			if (i == 0)
			{
				curRoomIndex = index;
				curRoomRight = (int)rooms.size() - 1;
				rooms[curRoomIndex].right = curRoomRight;
				rooms[curRoomRight].left = curRoomIndex;
				connections.emplace_back(curRoomIndex, curRoomRight);
			}
			else
			{
				curRoomIndex = (int)rooms.size() - 2;
				curRoomRight = (int)rooms.size() - 1;
				rooms[curRoomIndex].right = curRoomRight;
				rooms[curRoomRight].left = curRoomIndex;
				connections.emplace_back(curRoomIndex, curRoomRight);
			}

		}
	}
}

bool RoomLayout::setBoss(int numRooms)
{
	int left = -1;
	int bossIndex = rand() % (numRooms / 2) + numRooms / 2;
	int numTest = 0;
	while (rooms[bossIndex].left != -1 && rooms[bossIndex].right != -1)
	{
		if (++bossIndex > numRooms)
		{
			bossIndex = 1;
		}
		if (++numTest > numRooms) {
			return false;
		}
	}
	if (rooms[bossIndex].left == -1 && rooms[bossIndex].right == -1)
	{
		left = rand() % 2;
	}
	else if (rooms[bossIndex].left == -1)
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
	rooms[rooms.size() - 1].type = RoomData::Type::BOSS_ROOM;

	return true;
}

bool RoomLayout::setExit()
{
	int exitIndex = rand() % (rooms.size() - 1) + 1;
	int numTests = 0;
	while (rooms[exitIndex].type == RoomData::Type::BOSS_ROOM || rooms[exitIndex].type == RoomData::Type::START_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (rooms[exitIndex].left != -1 && rooms[rooms[exitIndex].left].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	while (rooms[exitIndex].right != -1 && rooms[rooms[exitIndex].right].type == RoomData::Type::BOSS_ROOM)
	{
		if (++exitIndex >= rooms.size())
		{
			exitIndex = 1;
		}
		if (++numTests > rooms.size()) {
			return false;
		}
	}
	rooms[exitIndex].type = RoomData::Type::EXIT_ROOM;

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
	rooms[one].shortcut = true;
	rooms[one].branchEnd = false;
	rooms[one].left = two;
	rooms[two].shortcut = true;
	rooms[two].branchEnd = false;
	rooms[two].right = one;

	return true;
}

int RoomLayout::getEndWithRightAvaliable()
{
	int ret = -1;
	for (int i = 0; i < rooms.size(); i++)
	{
		RoomData& curRoom = rooms[i];
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
		RoomData& curRoom = rooms[i];
		if (curRoom.branchEnd && curRoom.left == -1)
		{
			ret = i;
			break;
		}
	}
	return ret;
}

#if PLAY
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
#endif

bool RoomLayout::traverseRooms(int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta)
{
#if PLAY
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
#endif

	return false;
}

#if PLAY
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
#if PLAY
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
#endif
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
#endif