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

float getAngleBetween(const glm::vec3 one, const glm::vec3 two)
{
    float angle_between			= glm::degrees(glm::acos(glm::dot(one, two)));
    return angle_between;
}
