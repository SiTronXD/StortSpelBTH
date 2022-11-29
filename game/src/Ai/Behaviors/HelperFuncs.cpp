#include "HelperFuncs.hpp"


float lookAtY(const Transform& from, const Transform& to)
{
    float posX = from.position.x - to.position.x;
    float posZ = from.position.z - to.position.z;
    float angle = atan2(posX, posZ);
    angle = glm::degrees(angle);
  
    return angle; 
}
float lookAtY(const glm::vec3& from, const glm::vec3& to)
{
    float posX = from.x - to.x;
    float posZ = from.z - to.z;
    float angle = atan2(posX, posZ);
    angle = glm::degrees(angle);
  
    return angle; 
}

const glm::vec3 genRandomDir(const glm::vec3& manipulator)
{
    return glm::normalize(glm::vec3(
        1/(float)(rand()%100)   * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.x , 
        1/(float)(rand()%100)   * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.y , 
        (1/(float)(rand()%100)) * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.z ));
}

float getAngleBetween(const glm::vec3 one, const glm::vec3 two)
{
    float angle_between			= glm::degrees(glm::acos(glm::dot(one, two)));
    return angle_between;
}
