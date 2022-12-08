#pragma once
#include <glm/glm.hpp>
#include "vengine.h"


float           lookAtY(const Transform& from, const Transform& to);
float           lookAtY(const glm::vec3& from, const glm::vec3& to);
float           getAngleBetween(const glm::vec3 one, const glm::vec3 two);
const glm::vec3 genRandomDir(const glm::vec3& manipulator = {1.f,1.f,1.f});
void	        drawRaySimple(SceneHandler* sceneHandler, Ray& ray, float dist, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
void            avoidStuff(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir, bool drawRays = true);
void            avoidStuffBackwards(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir, bool drawRays = true);
glm::vec3       rotateVec(glm::vec3 rot, float deg, glm::vec3 axis);
glm::vec3       safeNormalize(glm::vec3& vec);
glm::vec2       safeNormalize(glm::vec2& vec);
glm::vec3       safeNormalize(glm::vec3&& vec);
glm::vec2       safeNormalize(glm::vec2&& vec);
glm::vec3       getDir(glm::vec3 from, glm::vec3 to);