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

void drawRaySimple(SceneHandler* sceneHandler, Ray& ray, float dist, glm::vec3 color)
{
	
	//Draw ray
	sceneHandler->getDebugRenderer()->renderLine(
	ray.pos,
	ray.pos + ray.dir * dist,
	glm::vec3(1.0f, 0.0f, 0.0f));

}

void avoidStuff(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir)
{
	bool somethingInTheWay = false;
	bool canGoForward=true;	
	bool canGoRight=true;
	bool canGoLeft=true;

	Collider& entityCollider = sceneHandler->getScene()->getComponent<Collider>(entityID);
	Transform& entityTransform = sceneHandler->getScene()->getComponent<Transform>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = entityTransform.position;
	glm::vec3 to = target;
	float maxDist = glm::length(from - to);
	glm::vec3 dir = glm::normalize(from - to);
	Ray rayToTarget{from, -dir};    
	Ray rayForward{from, -entityTransform.forward()};    
	Ray rayRight{from, entityTransform.right()};    
	Ray rayLeft{from, -entityTransform.right()};    
	Ray rayRightForward{from, glm::normalize(entityTransform.right()+rayForward.dir)};    
	Ray rayLeftForward{from, glm::normalize(-entityTransform.right()+rayForward.dir)}; 

	float left_right_maxDist = entityCollider.radius + 4.0f;
    //RayPayload rp = sceneHandler->getPhysicsEngine()->raycast(rayToTarget, maxDist);
	//drawRaySimple(sceneHandler, rayToTarget, maxDist);
	//if(rp.hit)
	//{
		//float len = glm::length(rp.hitPoint - entityTransform.position);
		//if(len > maxDist + 5.0f)
		//{

		//}
		//else if(!sceneHandler->getScene()->getComponent<Collider>(rp.entity).isTrigger &&
		//	rp.entity != entityID)
		//{
			
			somethingInTheWay = true;
			entityTransform.updateMatrix();

			RayPayload r_forward		= sceneHandler->getPhysicsEngine()->raycast(rayForward, left_right_maxDist);
			RayPayload r_right			= sceneHandler->getPhysicsEngine()->raycast(rayRight, left_right_maxDist);
			RayPayload r_left			= sceneHandler->getPhysicsEngine()->raycast(rayLeft, left_right_maxDist);
			RayPayload r_right_f		= sceneHandler->getPhysicsEngine()->raycast(rayRightForward, left_right_maxDist);
			RayPayload r_left_f			= sceneHandler->getPhysicsEngine()->raycast(rayLeftForward, left_right_maxDist);
			drawRaySimple(sceneHandler, rayForward, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));	
			drawRaySimple(sceneHandler, rayRight, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));			
			drawRaySimple(sceneHandler, rayLeft, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));
			drawRaySimple(sceneHandler, rayRightForward, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));
			drawRaySimple(sceneHandler, rayLeftForward, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));
			
			if(r_forward.hit && !sceneHandler->getScene()->getComponent<Collider>(r_forward.entity).isTrigger)
			{
				canGoForward = false;
			}
			if((r_right_f.hit && !sceneHandler->getScene()->getComponent<Collider>(r_right_f.entity).isTrigger) ||
				(r_right.hit && !sceneHandler->getScene()->getComponent<Collider>(r_right.entity).isTrigger))
			{
				canGoRight = false;
				attackGoRight = false;
			}
			if((r_left_f.hit && !sceneHandler->getScene()->getComponent<Collider>(r_left_f.entity).isTrigger) ||
				(r_left.hit && !sceneHandler->getScene()->getComponent<Collider>(r_left.entity).isTrigger))
			{
				canGoLeft = false;
				attackGoRight = true;
			}

		//}
	//}

	dir = glm::vec3(0.0f, 0.0f, 0.0f);
	if(somethingInTheWay)
	{
		
		entityTransform.updateMatrix();

		if(canGoForward)
		{
			dir += rayToTarget.dir;
		}
		if(canGoLeft && canGoRight && !canGoForward)
		{
			if(attackGoRight)
			{
				dir += entityTransform.right();
			}
			else
			{
				dir -= entityTransform.right();
			}
		}
		else if(canGoLeft && canGoRight)
		{
			dir = dir;
		}
		else if(canGoRight && attackGoRight)
		{
			dir += entityTransform.right();
		}
		else if(canGoLeft && !attackGoRight)
		{
			dir -= entityTransform.right();
		}
	}

	if(dir == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		dir = rayToTarget.dir;
	}
	glm::normalize(dir);
	dir.y = 0;
	wantedDir = dir;
}
