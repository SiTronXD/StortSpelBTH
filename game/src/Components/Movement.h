#pragma once

#include "glm/glm.hpp"

struct Movement
{
	glm::vec2 moveDir;
	glm::vec2 currentSpeed;
	float maxSpeed;
	float speedIncrease;
	float turnSpeed;
	float timer;
    float slowDown;
};