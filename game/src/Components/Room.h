#pragma once
#include "glm/glm.hpp"

struct Room
{
	enum ROOM_TYPE{START_ROOM, NORMAL_ROOM, HARD_ROOM, BOSS_ROOM, EXIT_ROOM};
	
	int up, down, left, right;
	glm::vec3 dimensions;
	ROOM_TYPE type;
	bool branch, branchEnd, shortcut;

	Room()
		:up(-1), down(-1), left(-1), right(-1),
		dimensions(glm::vec3(10.0f, 10.0f, 10.0f)),
		type(ROOM_TYPE::NORMAL_ROOM),branch(false), branchEnd(false),
		shortcut(false)
	{
	};

};


