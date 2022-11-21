#pragma once
#include <glm/glm.hpp>
#include "vengine.h"


float lookAtY(const Transform& from, const Transform& to);
float lookAtY(const glm::vec3& from, const glm::vec3& to);
float getAngleBetween(const glm::vec3 one, const glm::vec3 two);
