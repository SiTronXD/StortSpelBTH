#pragma once

#include "glm/glm.hpp"

struct AiMovement {
    glm::vec3 moveDir;
    glm::vec2 currentSpeed;
    float     maxSpeed;
    float     speedIncrease;
    float     turnSpeed;
    float     slowDown;
    float     distance;
};