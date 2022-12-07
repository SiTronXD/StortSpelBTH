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
    // Safety check, illegal to normalize a nullptr
    return safeNormalize(glm::vec3(
        1/(float)(rand()%100 + 1)   * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.x , 
        1/(float)(rand()%100 + 1)   * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.y , 
        1/(float)(rand()%100 + 1)   * (rand()%2 == 0 ? 1.f : -1.f) * manipulator.z 
    ));
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
	color);

}

void avoidStuff(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir, bool drawRays)
{
	bool somethingInTheWay = false;
	bool canGoForward=true;	
	bool canGoRight=true;
	bool canGoLeft=true;

	Collider& entityCollider = sceneHandler->getScene()->getComponent<Collider>(entityID);
	Transform& entityTransform = sceneHandler->getScene()->getComponent<Transform>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = entityTransform.position;
	from.y = 2.0f;
	glm::vec3 to = target;
	glm::vec3 dirToTarget =  safeNormalize(glm::vec3(to - entityTransform.position));
	float maxDist = glm::length(from - to);
	glm::vec3 dir = safeNormalize(from - to);   


	//Rays
	Ray ray0			{from, entityTransform.right()};    
	Ray ray30			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(30), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray60			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(60), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray90			{from, entityTransform.forward()};    
	Ray ray120			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(120), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray150			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(150), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray180			{from, -entityTransform.right()};    

	float left_right_maxDist = entityCollider.radius + 6.0f;

	//Payloads
	RayPayload r_0		        = sceneHandler->getPhysicsEngine()->raycast(ray0	, left_right_maxDist); //Right
	RayPayload r_30			    = sceneHandler->getPhysicsEngine()->raycast(ray30	, left_right_maxDist);	//Right
	RayPayload r_60 		    = sceneHandler->getPhysicsEngine()->raycast(ray60	, left_right_maxDist);	//Right
	RayPayload r_90		        = sceneHandler->getPhysicsEngine()->raycast(ray90	, left_right_maxDist);	//Forward
	RayPayload r_120    		= sceneHandler->getPhysicsEngine()->raycast(ray120	, left_right_maxDist);	//Left
	RayPayload r_150			= sceneHandler->getPhysicsEngine()->raycast(ray150	, left_right_maxDist);	//Left
	RayPayload r_180			= sceneHandler->getPhysicsEngine()->raycast(ray180	, left_right_maxDist);	//Left

        
	if(drawRays)
	{
        if(sceneHandler->getScene()->getNetworkHandler() != nullptr)
        {
      
            //Storing rays for drawing
            std::unordered_map<Ray*, RayPayload*>rays;
            rays.insert({&ray0,		&r_0});		
            rays.insert({&ray30,	&r_30});
            rays.insert({&ray60,	&r_60});
            rays.insert({&ray90,	&r_90});
            rays.insert({&ray120,	&r_120});
            rays.insert({&ray150,	&r_150});
            rays.insert({&ray180,	&r_180});

            for(auto r: rays)
            {
                if(r.second->hit)
                {
                    drawRaySimple(sceneHandler, *r.first	, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));	
                }
                else
                {
                    drawRaySimple(sceneHandler, *r.first	, left_right_maxDist, glm::vec3(1.0f, 0.0f, 0.0f));	
                }
            }
        }
	}
	
	
	if(r_90.hit && sceneHandler->getScene()->hasComponents<Collider>(r_90.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_90.entity).isTrigger)
	{
		canGoForward = false;
	}
	if((r_0.hit && sceneHandler->getScene()->hasComponents<Collider>(r_0.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_0.entity).isTrigger) ||
		(r_30.hit && sceneHandler->getScene()->hasComponents<Collider>(r_30.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_30.entity).isTrigger)||
		(r_60.hit && sceneHandler->getScene()->hasComponents<Collider>(r_60.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_60.entity).isTrigger))
	{
		canGoRight = false;
		attackGoRight = false;
	}
	if((r_120.hit && sceneHandler->getScene()->hasComponents<Collider>(r_120.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_120.entity).isTrigger) ||
		(r_150.hit && sceneHandler->getScene()->hasComponents<Collider>(r_150.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_150.entity).isTrigger)||
		(r_180.hit && sceneHandler->getScene()->hasComponents<Collider>(r_180.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_180.entity).isTrigger))
	{
		canGoLeft = false;
		attackGoRight = true;
	}

	somethingInTheWay = true;
	entityTransform.updateMatrix();

	dir = glm::vec3(0.0f, 0.0f, 0.0f);
	if(somethingInTheWay)
	{
		
		entityTransform.updateMatrix();

		if(canGoForward)
		{
			dir += dirToTarget;
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
		dir = dirToTarget;
	}
	glm::normalize(dir);
	dir.y = 0;
	wantedDir = dir;
}

void avoidStuffBackwards(Entity entityID, SceneHandler* sceneHandler, bool& attackGoRight, glm::vec3 target, glm::vec3& wantedDir, bool drawRays)
{
	bool somethingInTheWay = false;
	bool canGoForward=true;	
	bool canGoRight=true;
	bool canGoLeft=true;

	Collider& entityCollider = sceneHandler->getScene()->getComponent<Collider>(entityID);
	Transform& entityTransform = sceneHandler->getScene()->getComponent<Transform>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = entityTransform.position;
	from.y = 2.0f;
	glm::vec3 to = target;
	glm::vec3 dirToTarget =  safeNormalize(glm::vec3(to - entityTransform.position));
	float maxDist = glm::length(from - to);
	glm::vec3 dir = safeNormalize(to - from);   


	//Rays
	Ray ray0			{from, entityTransform.right()};    
	Ray ray30			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(-30), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray60			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(-60), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray90			{from, -entityTransform.forward()};    
	Ray ray120			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(-120), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray150			{from, rotateVec(entityTransform.right(), AI_DEG_TO_RAD(-150), glm::vec3(0.0f, 1.0f, 0.0f))};    
	Ray ray180			{from, -entityTransform.right()};    

	float left_right_maxDist = entityCollider.radius + 6.0f;

	//Payloads
	RayPayload r_0		        = sceneHandler->getPhysicsEngine()->raycast(ray0	, left_right_maxDist); //Right
	RayPayload r_30			    = sceneHandler->getPhysicsEngine()->raycast(ray30	, left_right_maxDist);	//Right
	RayPayload r_60 		    = sceneHandler->getPhysicsEngine()->raycast(ray60	, left_right_maxDist);	//Right
	RayPayload r_90		        = sceneHandler->getPhysicsEngine()->raycast(ray90	, left_right_maxDist);	//Forward
	RayPayload r_120    		= sceneHandler->getPhysicsEngine()->raycast(ray120	, left_right_maxDist);	//Left
	RayPayload r_150			= sceneHandler->getPhysicsEngine()->raycast(ray150	, left_right_maxDist);	//Left
	RayPayload r_180			= sceneHandler->getPhysicsEngine()->raycast(ray180	, left_right_maxDist);	//Left

        
	if(drawRays)
	{
        if(sceneHandler->getScene()->getNetworkHandler() != nullptr)
        {
      
            //Storing rays for drawing
            std::unordered_map<Ray*, RayPayload*>rays;
            rays.insert({&ray0,		&r_0});		
            rays.insert({&ray30,	&r_30});
            rays.insert({&ray60,	&r_60});
            rays.insert({&ray90,	&r_90});
            rays.insert({&ray120,	&r_120});
            rays.insert({&ray150,	&r_150});
            rays.insert({&ray180,	&r_180});

            for(auto r: rays)
            {
                if(r.second->hit)
                {
                    drawRaySimple(sceneHandler, *r.first	, left_right_maxDist, glm::vec3(0.0f, 1.0f, 0.0f));	
                }
                else
                {
                    drawRaySimple(sceneHandler, *r.first	, left_right_maxDist, glm::vec3(1.0f, 0.0f, 0.0f));	
                }
            }
        }
	}
	
	
	if(r_90.hit && sceneHandler->getScene()->hasComponents<Collider>(r_90.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_90.entity).isTrigger)
	{
		canGoForward = false;
	}
	if((r_0.hit && sceneHandler->getScene()->hasComponents<Collider>(r_0.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_0.entity).isTrigger) ||
		(r_30.hit && sceneHandler->getScene()->hasComponents<Collider>(r_30.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_30.entity).isTrigger)||
		(r_60.hit && sceneHandler->getScene()->hasComponents<Collider>(r_60.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_60.entity).isTrigger))
	{
		canGoRight = false;
		attackGoRight = false;

	}
	if((r_120.hit && sceneHandler->getScene()->hasComponents<Collider>(r_120.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_120.entity).isTrigger) ||
		(r_150.hit && sceneHandler->getScene()->hasComponents<Collider>(r_150.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_150.entity).isTrigger)||
		(r_180.hit && sceneHandler->getScene()->hasComponents<Collider>(r_180.entity) && !sceneHandler->getScene()->getComponent<Collider>(r_180.entity).isTrigger))
	{
		canGoLeft = false;
		attackGoRight = true;
	}

	somethingInTheWay = true;
	entityTransform.updateMatrix();

	dir = glm::vec3(0.0f, 0.0f, 0.0f);
	if(somethingInTheWay)
	{
		
		entityTransform.updateMatrix();

		if(canGoForward)
		{
			dir -= entityTransform.forward();
		}
		if(canGoLeft && canGoRight && !canGoForward)
		{
			if(attackGoRight)
			{
				dir -= entityTransform.right();
			}
			else
			{
				dir += entityTransform.right();
			}
		}
		else if(canGoLeft && canGoRight)
		{
			dir = dir;
		}
		else if(canGoRight && attackGoRight)
		{
			dir -= entityTransform.right();
		}
		else if(canGoLeft && !attackGoRight)
		{
			dir += entityTransform.right();
		}
	}

	if(dir == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		dir = -entityTransform.forward();
	}
	glm::normalize(dir);
	dir.y = 0;
	wantedDir = dir;
}

glm::vec3 rotateVec(glm::vec3 rot, float deg, glm::vec3 axis)
{
	glm::vec3 ret;
	glm::mat3x3 mat;
	glm::vec3 x = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 y = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
	if(axis == x)
	{
		mat = glm::mat3x3(
			{1,		0,			0},
			{0,		cos(deg),	-sin(deg)},
			{0,		sin(deg),	cos(deg)}
		);
	}
	else if(axis == y)
	{
		mat = glm::mat3x3(
			{cos(deg),	0,	sin(deg)},
			{0,			1,	0},
			{-sin(deg), 0,	cos(deg)}
		);
	}
	else if(axis == z)
	{
		mat = glm::mat3x3(
			{cos(deg),	-sin(deg),	0},
			{sin(deg),	cos(deg),	0},
			{0,			0,			1}
		);
	}
	else
	{
		std::cout<<"Error: Axis not valid.\n";
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	
	ret = mat*rot;

	return safeNormalize(ret);
}
glm::vec3 safeNormalize(glm::vec3& vec)
{
    if (glm::length(vec) != 0.f)
    {
        return glm::normalize(vec);
    }
    Log::warning("Tried to normalize zero vector; glm::vec3");
    return glm::vec3(1.f, 1.f, 1.f);
}

glm::vec2 safeNormalize(glm::vec2& vec)
{
    if (glm::length(vec) != 0.f)
    {
        return glm::normalize(vec);
    }
    Log::warning("Tried to normalize zero vector; glm::vec2");
    return glm::vec2(1.f, 1.f);
}
glm::vec3 safeNormalize(glm::vec3&& vec)
{
    if (glm::length(vec) != 0.f)
    {
        return glm::normalize(vec);
    }
    Log::warning("Tried to normalize zero vector; glm::vec3");
    return glm::vec3(1.f, 1.f, 1.f);
}

glm::vec2 safeNormalize(glm::vec2&& vec)
{
    if (glm::length(vec) != 0.f)
    {
        return glm::normalize(vec);
    }
    Log::warning("Tried to normalize zero vector; glm::vec2");
    return glm::vec2(1.f, 1.f);
}