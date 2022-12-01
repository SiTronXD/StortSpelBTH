#pragma once
#include <glm/glm.hpp>
#include "vengine.h"


float       lookAtY(const Transform& from, const Transform& to);
float       lookAtY(const glm::vec3& from, const glm::vec3& to);
float       getAngleBetween(const glm::vec3 one, const glm::vec3 two);
void	    drawRaySimple(SceneHandler* sceneHandler, Ray& ray, float dist, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
void        avoidStuff(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir, glm::vec3 rayOriginOffset = glm::vec3(0.0f, 0.0f, 0.0f), bool drawRays = false);
glm::vec3   rotateVec(glm::vec3 rot, float deg, glm::vec3 axis);